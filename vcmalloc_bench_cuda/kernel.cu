
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <iostream>
#include "vcmalloc.h"
#include <mimalloc/include/mimalloc.h>
#include "json/json.h"
#include <string>
#include <pcm/src/cpucounters.h>
#include <psapi.h>
//#include <windows.h>

using namespace std;
using namespace pcm;
using namespace vcma;


__global__ void vecAdd(double* a, double* b, double* c)
{

    int id = blockIdx.x;
    c[id] = a[id] + b[id];
}

__global__ void vecAdd_compact(double* data)
{
    double* a = data;
    double* b = data + gridDim.x;
    double* c = data + gridDim.x*2;

    int id = blockIdx.x;
    c[id] = a[id] + b[id];
}

void inline cuda_check() {
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        printf("Error: %s\n", cudaGetErrorString(err));
        throw std::string("We have a problem");
    }
}


void to_json(Json::Value n_rslt, int n_test) {
    Json::Value rslt;

    std::fstream rslt_file;
    rslt_file.open("results.json", std::ios::in);

    //Read current json file
    Json::CharReaderBuilder rbuilder;
    rbuilder["collectComments"] = false;
    std::string errs;
    bool ok = Json::parseFromStream(rbuilder, rslt_file, &rslt, &errs);
    rslt_file.close();

    //Add new data

    string scenario = n_rslt["scenario"].asString();
    string allocator = n_rslt["allocator"].asString();
    string datatype = n_rslt["datatype"].asString();
    string size = n_rslt["size"].asString();

    auto operations = n_rslt["operations"].getMemberNames();

    for (auto operation : operations) {
        auto results = n_rslt["operations"][operation].getMemberNames();
        for (auto result : results) {
            rslt[scenario][allocator][datatype][size][operation][result][to_string(n_test)] = n_rslt["operations"][operation][result].asString();
        }
    }

    //Write file
    rslt_file.open("results.json", std::ios::out);
    rslt_file << rslt.toStyledString() << std::endl;
    rslt_file.close();

    cout << scenario << " " << allocator << " " << datatype << " " << size << " " << n_test << endl;
}

void result_filler(
    Json::Value& result,
    std::string operation,
    clock_t c_before, clock_t c_after,
    SystemCounterState pcm_before, SystemCounterState pcm_after,
    PROCESS_MEMORY_COUNTERS ps_before, PROCESS_MEMORY_COUNTERS ps_after,
    cudaEvent_t cu_before, cudaEvent_t cu_after) {

    result["operations"][operation]["Memory Consumption"] = ps_after.PagefileUsage - ps_before.PagefileUsage;

    result["operations"][operation]["Cycles"] = (int)(c_after - c_before);

    //result["operations"][operation]["PCM Cycles"]				=	getCycles(pcm_before, pcm_after);
    //result["operations"][operation]["IPC"]						=	getIPC(pcm_before, pcm_after);
    result["operations"][operation]["L2 Cache Misses"] = getL2CacheMisses(pcm_before, pcm_after);
    //result["operations"][operation]["L2 Cache Hits"]			=	getL2CacheHits(pcm_before, pcm_after);
    //result["operations"][operation]["L2 Cache Hit Ratio"]		=	getL2CacheHitRatio(pcm_before, pcm_after);
    result["operations"][operation]["L3 Cache Misses"] = getL3CacheMisses(pcm_before, pcm_after);
    //result["operations"][operation]["L3 Cache Hits"]			=	getL3CacheHits(pcm_before, pcm_after);
    //result["operations"][operation]["L3 Cache Hit Ratio"]		=	getL3CacheHitRatio(pcm_before, pcm_after);
    result["operations"][operation]["Consumed Joules"] = getConsumedJoules(pcm_before, pcm_after);
    //result["operations"][operation]["DRAM Consumed Joules"]		=	getDRAMConsumedJoules(pcm_before, pcm_after);

    float time;
    cudaEventElapsedTime(&time, cu_before, cu_after);
    result["operations"][operation]["CUDA Time"] = time;


}


Json::Value vcmalloc_cuda_copy(SIZE_T param_num) {

    SIZE_T allocSize = pow(2, 27);
    SIZE_T N = sqrt((allocSize / param_num)) / sqrt(sizeof(double));
    SIZE_T M = ((allocSize / param_num) / (N*sizeof(double)));

    Json::Value result;
    result["allocator"] = "vcm";
    result["scenario"] = "CUDA";
    result["datatype"] = typeid(double).name();
    result["size"] = param_num;


    
    //evaluation variables
    clock_t c_before, c_after;						//clock
    SystemCounterState pcm_before, pcm_after;		//intel pcm
    PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status
    cudaEvent_t cu_before, cu_after;
    cudaEventCreate(&cu_before);
    cudaEventCreate(&cu_after);

    
    double **h_data;
    double *d_data;

    void* vc = vccalloc((SIZE_T)1024 * 1024 * 1024 * 1024);
    vcmalloc(vc, h_data, param_num);
    


    //Host allocation
    for (size_t i = 0; i < param_num; i++)
        vcmalloc(vc, h_data[i], N * M);
    
    // Host initialization
    for (SIZE_T i = 0; i < param_num; i++)
        for (size_t j = 0; j < N * M; j++)
            h_data[i][j] = i * N * M + j;

    // Device allocation
    cudaMalloc((void**)&d_data, sizeof(double) * N * M * param_num);
    cuda_check();

    // Transfer data from host to device memory
    //before
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
    pcm_before = getSystemCounterState();
    c_before = clock();
    cudaEventRecord(cu_before, 0);

    cudaMemcpy(d_data, *h_data, sizeof(double) * N * M * param_num, cudaMemcpyHostToDevice);
    cuda_check();

    //after
    cudaEventRecord(cu_after, 0);
    cudaEventSynchronize(cu_after);
    c_after = clock();
    pcm_after = getSystemCounterState();
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
    result_filler(result, "Host to Device", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after, cu_before, cu_after);
    
    // Launch the calculation
    // vecAdd<<<M*N, 1>>> (&d_data[0], &d_data[N*M], &d_data[N*M*2]);
    // vecAdd_compact<<<M*N, 1>>> (d_data);
    //cudaDeviceSynchronize();

    //Transfer data from device to host
    //before
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
    pcm_before = getSystemCounterState();
    c_before = clock();
    cudaEventRecord(cu_before, 0);

    cudaMemcpy(h_data[param_num-1], &d_data[M*N*(param_num-1)], sizeof(double) * N * M, cudaMemcpyDeviceToHost);
    cuda_check();

    //after
    cudaEventRecord(cu_after, 0);
    cudaEventSynchronize(cu_after);
    c_after = clock();
    pcm_after = getSystemCounterState();
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
    result_filler(result, "Device to Host", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after, cu_before, cu_after);

    //std::cout << "c[" << N * M - 1 << "]: " << h_c[1] << std::endl;

    // Cleanup after kernel execution
    cudaFree(d_data);
    vccfree(vc);

    return result;
    
}
Json::Value malloc_cuda_copy(SIZE_T param_num) {

    SIZE_T allocSize = pow(2, 27);
    SIZE_T N = sqrt((allocSize / param_num)) / sqrt(sizeof(double));
    SIZE_T M = ((allocSize / param_num) / (N * sizeof(double)));

    Json::Value result;
    result["allocator"] = "m";
    result["scenario"] = "CUDA";
    result["datatype"] = typeid(double).name();
    result["size"] = param_num;

    double** h_data = (double**)malloc(sizeof(double*) * param_num);
    double** d_data = (double**)malloc(sizeof(double*) * param_num);


    //evaluation variables
    clock_t c_before, c_after;						//clock
    SystemCounterState pcm_before, pcm_after;		//intel pcm
    PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status
    cudaEvent_t cu_before, cu_after;
    cudaEventCreate(&cu_before);
    cudaEventCreate(&cu_after);



    //Host allocation
    for (size_t i = 0; i < param_num; i++)
        h_data[i] = (double*)malloc(sizeof(double) * N * M);

    // Host initialization
    for (SIZE_T i = 0; i < param_num; i++)
        for (size_t j = 0; j < N * M; j++)
            h_data[i][j] = i * N * M + j;

    // Device allocation
    for (size_t i = 0; i < param_num; i++) {
        cudaMalloc((void**)&d_data[i], sizeof(double) * N * M);
        cuda_check();
    }

    // Transfer data from host to device memory
    //before
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
    pcm_before = getSystemCounterState();
    c_before = clock();
    cudaEventRecord(cu_before, 0);

    for (size_t i = 0; i < param_num; i++){
       cudaMemcpy(d_data[i], h_data[i], sizeof(double) * N * M, cudaMemcpyHostToDevice);
       cuda_check();
    }
    //after
    cudaEventRecord(cu_after, 0);
    cudaEventSynchronize(cu_after);
    c_after = clock();
    pcm_after = getSystemCounterState();
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
    result_filler(result, "Host to Device", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after, cu_before, cu_after);
    
    //Launch the calculation
    //vecAdd<<<M*N, 1>>> (&d_data[0], &d_data[N*M], &d_data[N*M*2]);
    //vecAdd_compact<<<M*N, 1>>> (d_data);
    //cudaDeviceSynchronize();

    //Transfer data from device to host
    //before
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
    pcm_before = getSystemCounterState();
    c_before = clock();
    cudaEventRecord(cu_before, 0);

    cudaMemcpy(h_data[param_num-1], d_data[param_num - 1], sizeof(double) * N * M, cudaMemcpyDeviceToHost);
    cuda_check();

    //after
    cudaEventRecord(cu_after, 0);
    cudaEventSynchronize(cu_after);
    c_after = clock();
    pcm_after = getSystemCounterState();
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
    result_filler(result, "Device to Host", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after, cu_before, cu_after);

    //std::cout << "c[" << N * M - 1 << "]: " << h_c[1] << std::endl;

    // Cleanup after kernel execution
    for (size_t i = 0; i < param_num; i++)
    {
        cudaFree(d_data[i]);
        free(h_data[i]);
    }

    return result;
    
}
Json::Value mi_malloc_cuda_copy(SIZE_T param_num) {

    SIZE_T allocSize = pow(2, 27);
    SIZE_T N = sqrt((allocSize / param_num)) / sqrt(sizeof(double));
    SIZE_T M = ((allocSize / param_num) / (N * sizeof(double)));

    Json::Value result;
    result["allocator"] = "mim";
    result["scenario"] = "CUDA";
    result["datatype"] = typeid(double).name();
    result["size"] = param_num;

    double** h_data = (double**)mi_malloc(sizeof(double*) * param_num);
    double** d_data = (double**)mi_malloc(sizeof(double*) * param_num);


    //evaluation variables
    clock_t c_before, c_after;						//clock
    SystemCounterState pcm_before, pcm_after;		//intel pcm
    PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status
    cudaEvent_t cu_before, cu_after;
    cudaEventCreate(&cu_before);
    cudaEventCreate(&cu_after);



    //Host allocation
    for (size_t i = 0; i < param_num; i++)
        h_data[i] = (double*)mi_malloc(sizeof(double) * N * M);

    // Host initialization
    for (SIZE_T i = 0; i < param_num; i++)
        for (size_t j = 0; j < N * M; j++)
            h_data[i][j] = i * N * M + j;

    // Device allocation
    for (size_t i = 0; i < param_num; i++) {
        cudaMalloc((void**)&d_data[i], sizeof(double) * N * M);
        cuda_check();
    }

    // Transfer data from host to device memory
    //before
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
    pcm_before = getSystemCounterState();
    c_before = clock();
    cudaEventRecord(cu_before, 0);

    for (size_t i = 0; i < param_num; i++) {
        cudaMemcpy(d_data[i], h_data[i], sizeof(double) * N * M, cudaMemcpyHostToDevice);
        cuda_check();
    }
    //after
    cudaEventRecord(cu_after, 0);
    cudaEventSynchronize(cu_after);
    c_after = clock();
    pcm_after = getSystemCounterState();
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
    result_filler(result, "Host to Device", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after, cu_before, cu_after);

    //Launch the calculation
    //vecAdd<<<M*N, 1>>> (&d_data[0], &d_data[N*M], &d_data[N*M*2]);
    //vecAdd_compact<<<M*N, 1>>> (d_data);
    //cudaDeviceSynchronize();

    //Transfer data from device to host
    //before
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
    pcm_before = getSystemCounterState();
    c_before = clock();
    cudaEventRecord(cu_before, 0);

    cudaMemcpy(h_data[param_num - 1], d_data[param_num - 1], sizeof(double) * N * M, cudaMemcpyDeviceToHost);
    cuda_check();

    //after
    cudaEventRecord(cu_after, 0);
    cudaEventSynchronize(cu_after);
    c_after = clock();
    pcm_after = getSystemCounterState();
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
    result_filler(result, "Device to Host", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after, cu_before, cu_after);

    //std::cout << "c[" << N * M - 1 << "]: " << h_c[1] << std::endl;

    // Cleanup after kernel execution
    for (size_t i = 0; i < param_num; i++)
    {
        cudaFree(d_data[i]);
        mi_free(h_data[i]);
    }

    return result;

}


Json::Value vcmalloc_cuda_pin(SIZE_T param_num) {

    SIZE_T allocSize = pow(2, 27);
    SIZE_T N = sqrt((allocSize / param_num)) / sqrt(sizeof(double));
    SIZE_T M = ((allocSize / param_num) / (N * sizeof(double)));

    Json::Value result;
    result["allocator"] = "vcm";
    result["scenario"] = "CUDA";
    result["datatype"] = typeid(double).name();
    result["size"] = param_num;



    //evaluation variables
    clock_t c_before, c_after;						//clock
    SystemCounterState pcm_before, pcm_after;		//intel pcm
    PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status
    cudaEvent_t cu_before, cu_after;
    cudaEventCreate(&cu_before);
    cudaEventCreate(&cu_after);


    double** h_data;
    double* d_data;
    
    void* vc = vccalloc((SIZE_T)1024 * 1024 * 1024 * 1024);
    vcmalloc(vc, h_data, param_num);

    //Host allocation
    for (size_t i = 0; i < param_num; i++)
        vcmalloc(vc, h_data[i], N * M);

    // Host initialization
    for (SIZE_T i = 0; i < param_num; i++)
        for (size_t j = 0; j < N * M; j++)
            h_data[i][j] = i * N * M + j;

    // Device allocation
    cudaMalloc((void**)&d_data, sizeof(double) * N * M * param_num);
    cuda_check();


    // Memory Pinning
    //before
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
    pcm_before = getSystemCounterState();
    c_before = clock();
    cudaEventRecord(cu_before, 0);

    cudaHostRegister(*h_data, sizeof(double) * N * M * param_num, cudaHostRegisterPortable);
    cuda_check();

    //after
    cudaEventRecord(cu_after, 0);
    cudaEventSynchronize(cu_after);
    c_after = clock();
    pcm_after = getSystemCounterState();
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
    result_filler(result, "Pinning", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after, cu_before, cu_after);

    // Transfer data from host to device memory
    //before
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
    pcm_before = getSystemCounterState();
    c_before = clock();
    cudaEventRecord(cu_before, 0);

    cudaMemcpy(d_data, *h_data, sizeof(double) * N * M * param_num, cudaMemcpyHostToDevice);
    cuda_check();

    //after
    cudaEventRecord(cu_after, 0);
    cudaEventSynchronize(cu_after);
    c_after = clock();
    pcm_after = getSystemCounterState();
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
    result_filler(result, "Host to Device (Pinned)", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after, cu_before, cu_after);

    // Launch the calculation
    // vecAdd<<<M*N, 1>>> (&d_data[0], &d_data[N*M], &d_data[N*M*2]);
    // vecAdd_compact<<<M*N, 1>>> (d_data);
    //cudaDeviceSynchronize();

    //Transfer data from device to host
    //before
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
    pcm_before = getSystemCounterState();
    c_before = clock();
    cudaEventRecord(cu_before, 0);

    cudaMemcpy(h_data[param_num - 1], &d_data[M * N * (param_num - 1)], sizeof(double) * N * M, cudaMemcpyDeviceToHost);
    cuda_check();

    //after
    cudaEventRecord(cu_after, 0);
    cudaEventSynchronize(cu_after);
    c_after = clock();
    pcm_after = getSystemCounterState();
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
    result_filler(result, "Device to Host (Pinned)", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after, cu_before, cu_after);

    //std::cout << "c[" << N * M - 1 << "]: " << h_c[1] << std::endl;

    // Cleanup after kernel execution
    cudaFree(d_data);
    vccfree(vc);

    return result;

}
Json::Value malloc_cuda_pin(SIZE_T param_num) {

    SIZE_T allocSize = pow(2, 27);
    SIZE_T N = sqrt((allocSize / param_num)) / sqrt(sizeof(double));
    SIZE_T M = ((allocSize / param_num) / (N * sizeof(double)));

    Json::Value result;
    result["allocator"] = "m";
    result["scenario"] = "CUDA";
    result["datatype"] = typeid(double).name();
    result["size"] = param_num;

    double** h_data = (double**)malloc(sizeof(double*) * param_num);
    double** d_data = (double**)malloc(sizeof(double*) * param_num);


    //evaluation variables
    clock_t c_before, c_after;						//clock
    SystemCounterState pcm_before, pcm_after;		//intel pcm
    PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status
    cudaEvent_t cu_before, cu_after;
    cudaEventCreate(&cu_before);
    cudaEventCreate(&cu_after);



    //Host allocation
    for (size_t i = 0; i < param_num; i++)
        h_data[i] = (double*)malloc(sizeof(double) * N * M);

    // Host initialization
    for (SIZE_T i = 0; i < param_num; i++)
        for (size_t j = 0; j < N * M; j++)
            h_data[i][j] = i * N * M + j;

    // Device allocation
    for (size_t i = 0; i < param_num; i++) {
        cudaMalloc((void**)&d_data[i], sizeof(double) * N * M);
        cuda_check();
    }


    // Memory Pinning
    //before
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
    pcm_before = getSystemCounterState();
    c_before = clock();
    cudaEventRecord(cu_before, 0);

    for (size_t i = 0; i < param_num; i++) {
        cudaHostRegister(h_data[i], sizeof(double) * N * M, cudaHostRegisterPortable);
        cuda_check();
    }
    //after
    cudaEventRecord(cu_after, 0);
    cudaEventSynchronize(cu_after);
    c_after = clock();
    pcm_after = getSystemCounterState();
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
    result_filler(result, "Pinning", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after, cu_before, cu_after);



    // Transfer data from host to device memory
    //before
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
    pcm_before = getSystemCounterState();
    c_before = clock();
    cudaEventRecord(cu_before, 0);

    for (size_t i = 0; i < param_num; i++) {
        cudaMemcpy(d_data[i], h_data[i], sizeof(double) * N * M, cudaMemcpyHostToDevice);
        cuda_check();
    }
    //after
    cudaEventRecord(cu_after, 0);
    cudaEventSynchronize(cu_after);
    c_after = clock();
    pcm_after = getSystemCounterState();
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
    result_filler(result, "Host to Device (Pinned)", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after, cu_before, cu_after);

    // Launch the calculation
    // vecAdd<<<M*N, 1>>> (&d_data[0], &d_data[N*M], &d_data[N*M*2]);
    // vecAdd_compact<<<M*N, 1>>> (d_data);
    //cudaDeviceSynchronize();

    //Transfer data from device to host
    //before
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
    pcm_before = getSystemCounterState();
    c_before = clock();
    cudaEventRecord(cu_before, 0);

    cudaMemcpy(h_data[param_num - 1], d_data[param_num - 1], sizeof(double) * N * M, cudaMemcpyDeviceToHost);
    cuda_check();

    //after
    cudaEventRecord(cu_after, 0);
    cudaEventSynchronize(cu_after);
    c_after = clock();
    pcm_after = getSystemCounterState();
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
    result_filler(result, "Device to Host (Pinned)", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after, cu_before, cu_after);

    //std::cout << "c[" << N * M - 1 << "]: " << h_c[1] << std::endl;

    // Cleanup after kernel execution
    for (size_t i = 0; i < param_num; i++)
    {
        cudaFree(d_data[i]);
        free(h_data[i]);
    }

    return result;

}
Json::Value mi_malloc_cuda_pin(SIZE_T param_num) {

    SIZE_T allocSize = pow(2, 27);
    SIZE_T N = sqrt((allocSize / param_num)) / sqrt(sizeof(double));
    SIZE_T M = ((allocSize / param_num) / (N * sizeof(double)));

    Json::Value result;
    result["allocator"] = "mim";
    result["scenario"] = "CUDA";
    result["datatype"] = typeid(double).name();
    result["size"] = param_num;

    double** h_data = (double**)mi_malloc(sizeof(double*) * param_num);
    double** d_data = (double**)mi_malloc(sizeof(double*) * param_num);


    //evaluation variables
    clock_t c_before, c_after;						//clock
    SystemCounterState pcm_before, pcm_after;		//intel pcm
    PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status
    cudaEvent_t cu_before, cu_after;
    cudaEventCreate(&cu_before);
    cudaEventCreate(&cu_after);



    //Host allocation
    for (size_t i = 0; i < param_num; i++)
        h_data[i] = (double*)mi_malloc(sizeof(double) * N * M);

    // Host initialization
    for (SIZE_T i = 0; i < param_num; i++)
        for (size_t j = 0; j < N * M; j++)
            h_data[i][j] = i * N * M + j;

    // Device allocation
    for (size_t i = 0; i < param_num; i++) {
        cudaMalloc((void**)&d_data[i], sizeof(double) * N * M);
        cuda_check();
    }


    // Memory Pinning
    //before
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
    pcm_before = getSystemCounterState();
    c_before = clock();
    cudaEventRecord(cu_before, 0);

    for (size_t i = 0; i < param_num; i++) {
        cudaHostRegister(h_data[i], sizeof(double) * N * M, cudaHostRegisterPortable);
        cuda_check();
    }
    //after
    cudaEventRecord(cu_after, 0);
    cudaEventSynchronize(cu_after);
    c_after = clock();
    pcm_after = getSystemCounterState();
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
    result_filler(result, "Pinning", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after, cu_before, cu_after);



    // Transfer data from host to device memory
    //before
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
    pcm_before = getSystemCounterState();
    c_before = clock();
    cudaEventRecord(cu_before, 0);

    for (size_t i = 0; i < param_num; i++) {
        cudaMemcpy(d_data[i], h_data[i], sizeof(double) * N * M, cudaMemcpyHostToDevice);
        cuda_check();
    }
    //after
    cudaEventRecord(cu_after, 0);
    cudaEventSynchronize(cu_after);
    c_after = clock();
    pcm_after = getSystemCounterState();
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
    result_filler(result, "Host to Device (Pinned)", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after, cu_before, cu_after);

    // Launch the calculation
    // vecAdd<<<M*N, 1>>> (&d_data[0], &d_data[N*M], &d_data[N*M*2]);
    // vecAdd_compact<<<M*N, 1>>> (d_data);
    //cudaDeviceSynchronize();

    //Transfer data from device to host
    //before
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
    pcm_before = getSystemCounterState();
    c_before = clock();
    cudaEventRecord(cu_before, 0);

    cudaMemcpy(h_data[param_num - 1], d_data[param_num - 1], sizeof(double) * N * M, cudaMemcpyDeviceToHost);
    cuda_check();

    //after
    cudaEventRecord(cu_after, 0);
    cudaEventSynchronize(cu_after);
    c_after = clock();
    pcm_after = getSystemCounterState();
    GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
    result_filler(result, "Device to Host (Pinned)", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after, cu_before, cu_after);

    //std::cout << "c[" << N * M - 1 << "]: " << h_c[1] << std::endl;

    // Cleanup after kernel execution
    for (size_t i = 0; i < param_num; i++)
    {
        cudaFree(d_data[i]);
        mi_free(h_data[i]);
    }

    return result;

}


int main(int argc, char* argv[]) {

    PCM* m = PCM::getInstance();
    m->resetPMU();
    if (m->program() != PCM::Success) return 0;

    typedef Json::Value(*tests_f)(SIZE_T allocSize);
    tests_f tests[] = {
        /*0*/	&vcmalloc_cuda_copy,
        /*1*/	&malloc_cuda_copy,
        /*2*/	&mi_malloc_cuda_copy,
        /*3*/   &vcmalloc_cuda_pin,
        /*4*/   &malloc_cuda_pin,
        /*5*/   &mi_malloc_cuda_pin

    };

    int test_id = stoi(argv[1]);
    int size_choice = stoi(argv[2]);
    int nbr_of_test = stoi(argv[3]);
    //SIZE_T size = pow(2, size_choice);

    to_json(tests[test_id](size_choice), nbr_of_test);

    m->cleanup();

    return 0;
}
