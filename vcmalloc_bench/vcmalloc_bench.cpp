#include <iostream>
#include <cmath>
#include <string>
#include <fstream>
#include <time.h>

#include <vcmalloc.h>
#include<mimalloc/include/mimalloc.h>

#include <pcm/src/cpucounters.h>
#include "json/json.h"

#include <data_structures/orderedLinkedList.h>
#include <data_structures/unorderedLinkedList.h>
#include <data_structures/stackUnorderedLinkedList.h>
#include <data_structures/doublyLinkedList.h>
#include <data_structures/linkedQueue.h>
#include <data_structures/linkedStack.h>
#include <data_structures/binarySearchTree.h>
#include <psapi.h>


using namespace vcma;
using namespace pcm;

void to_json(Json::Value n_rslt, int n_test, std::string fileName) {
	Json::Value rslt;

	std::fstream rslt_file;
	rslt_file.open(fileName, std::ios::in);

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
	rslt_file.open(fileName, std::ios::out);
	rslt_file << rslt.toStyledString() << std::endl;
	rslt_file.close();

	cout << scenario << " " << allocator << " " << datatype << " " << size << " " << n_test << endl;
}

void result_filler(
	Json::Value& result,
	std::string operation,
	clock_t c_before, clock_t c_after,
	SystemCounterState pcm_before, SystemCounterState pcm_after,
	PROCESS_MEMORY_COUNTERS ps_before, PROCESS_MEMORY_COUNTERS ps_after) {

	result["operations"][operation]["Memory Consumption"] = ps_after.PagefileUsage - ps_before.PagefileUsage;
	result["operations"][operation]["Cycles"] = (int)(c_after - c_before);
	//result["operations"][operation]["PCM Cycles"]				=	getCycles(pcm_before, pcm_after);
	//result["operations"][operation]["IPC"]					=	getIPC(pcm_before, pcm_after);
	result["operations"][operation]["L2 Cache Misses"] = getL2CacheMisses(pcm_before, pcm_after);
	//result["operations"][operation]["L2 Cache Hits"]			=	getL2CacheHits(pcm_before, pcm_after);
	//result["operations"][operation]["L2 Cache Hit Ratio"]		=	getL2CacheHitRatio(pcm_before, pcm_after);
	result["operations"][operation]["L3 Cache Misses"] = getL3CacheMisses(pcm_before, pcm_after);
	//result["operations"][operation]["L3 Cache Hits"]			=	getL3CacheHits(pcm_before, pcm_after);
	//result["operations"][operation]["L3 Cache Hit Ratio"]		=	getL3CacheHitRatio(pcm_before, pcm_after);
	result["operations"][operation]["Consumed Joules"] = getConsumedJoules(pcm_before, pcm_after);
	//result["operations"][operation]["DRAM Consumed Joules"]	=	getDRAMConsumedJoules(pcm_before, pcm_after);

}

/*
template<typename HeadType, typename NodeType> Json::Value vcmalloc_separated(SIZE_T allocSize) {
	allocSize = pow(2, allocSize);

	SIZE_T heads = sqrt(allocSize) / sqrt(sizeof(HeadType));
	SIZE_T nodes = ((allocSize / heads) - sizeof(HeadType)) / sizeof(NodeType);

	cout << "Heads: " << heads << endl;
	cout << "Nodess: " << nodes << endl;

	Json::Value result;
	result["allocator"] = "VCMalloc (Multiple Containers)";
	result["scenario"] = "Linked Data Structures";
	result["datatype"] = typeid(HeadType).name();
	result["size"] = allocSize;

	//evaluation variables
	clock_t c_before, c_after;						//clock
	SystemCounterState pcm_before, pcm_after;		//intel pcm
	PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status


	// allocation + first access
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	PVOID control_container = vccalloc((SIZE_T)sizeof(void*) * heads * 3);
	ExtendCcStorage(control_container, 3);
	ExtendPfnsStorage(control_container, sizeof(void*)*heads*3);

	void** container;
	vcmalloc(control_container, container, heads);

	for (SIZE_T i = 0; i < heads; i++) {
		container[i] = (ULONG_PTR*)vccalloc((SIZE_T)sizeof(HeadType) + sizeof(NodeType)*nodes);
		ExtendCcStorage(container[i], 2);
		ExtendPfnsStorage(container[i], sizeof(HeadType) + sizeof(NodeType) * nodes);
	}



	HeadType** headsP;
	NodeType** nodesP;
	vcmalloc(control_container, headsP, heads);
	vcmalloc(control_container, nodesP, heads);
	for (SIZE_T i = 0; i < heads; i++)
	{
		vcmalloc(container[i], headsP[i], 1);
		vcmalloc(container[i], nodesP[i], nodes);

		new(headsP[i]) HeadType();

		for (SIZE_T j = 0; j < nodes; j++)
		{
			headsP[i]->perTypeAppend(i * nodes + j, nodesP[i]+j);
		}
	}

	for (SIZE_T i = 0; i < heads; i++)
	{
		if (headsP[i]->search(-1))
			system("pause");
	}

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Allocation + Linear First Access", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);



	//Second Access
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
	{
		if (headsP[i]->search(-1))
			system("pause");
	}

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Linear Second Access", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);

	return result;

}
*/
/*
template<typename HeadType, typename NodeType> Json::Value vcmalloc_separated(SIZE_T allocSize) {
	allocSize = pow(2, allocSize);
	//SIZE_T heads = sqrt(allocSize) / sizeof(HeadType);
	SIZE_T heads = sqrt(allocSize) / sqrt(sizeof(HeadType));
	SIZE_T container_size = allocSize / heads;
	SIZE_T nodes = (container_size - sizeof(HeadType)) / (sizeof(NodeType));


	cout << "Heads: " << heads << endl;
	cout << "Nodess: " << nodes << endl;

	Json::Value result;
	result["allocator"] = "VCMalloc (Seperated Containers)";
	result["scenario"] = "Linked Data Structures";
	result["datatype"] = typeid(HeadType).name();
	result["size"] = allocSize;


	//evaluation variables
	clock_t c_before, c_after;						//clock
	SystemCounterState pcm_before, pcm_after;		//intel pcm
	PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status


	// allocation + first access

	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before	=	getSystemCounterState();
	c_before	=	clock();

	void* c_pointers = vccalloc((SIZE_T)2 * sizeof(void*) * heads);
	ExtendCcStorage(c_pointers,2);
	ExtendPfnsStorage(c_pointers, (SIZE_T)2 * sizeof(void*) * heads);


	void* c_data = vccalloc((SIZE_T)(sizeof(HeadType) + sizeof(NodeType) * nodes) * heads);
	ExtendCcStorage(c_data, 2*heads);
	ExtendPfnsStorage(c_data, (SIZE_T)(sizeof(HeadType) + sizeof(NodeType) * nodes) * heads);

	HeadType** headsP;
	NodeType** nodesP;



	vcmalloc(c_pointers, headsP, heads);
	vcmalloc(c_pointers, nodesP, heads);

	for (SIZE_T i = 0; i < heads; i++)
	{
		vcmalloc(c_data, headsP[i], 1);
		vcmalloc(c_data, nodesP[i], nodes);
		new(headsP[i]) HeadType();

		for (SIZE_T j = 0; j < nodes; j++) {
			headsP[i]->perTypeAppend(i * nodes + j, &nodesP[i][j]);
		}
	}

	for (SIZE_T i = 0; i < heads; i++)
	{
		if (headsP[i]->search(-1))
			cout << "ERROR!!!";
	}

	//after
	c_after		=	clock();
	pcm_after	=	getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Allocation + Linear First Access", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);


	//Second access

	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
	{
		if (headsP[i]->search(-1))
			cout << "ERROR!!!";
	}

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Linear Second Access", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);


	return result;

}
*/

template<typename HeadType, typename NodeType> Json::Value vcmalloc_single(SIZE_T allocSize) {
	allocSize = pow(2, allocSize);
	//SIZE_T heads = sqrt(allocSize) / sizeof(HeadType);
	SIZE_T heads = sqrt(allocSize) / sqrt(sizeof(HeadType));
	SIZE_T nodes = ((allocSize / heads) - sizeof(HeadType)) / (sizeof(NodeType));


	cout << "Heads: " << heads << endl;
	cout << "Nodess: " << nodes << endl;

	Json::Value result;
	result["allocator"] = "vcm";
	result["scenario"] = "Linked Data Structures";
	result["datatype"] = typeid(HeadType).name();
	result["size"] = allocSize;


	//evaluation variables
	clock_t c_before, c_after;						//clock
	SystemCounterState pcm_before, pcm_after;		//intel pcm
	PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status


	// allocation + first access

	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	SIZE_T container_size = (SIZE_T)2 * sizeof(void*) * heads + (SIZE_T)(sizeof(HeadType) + sizeof(NodeType) * nodes) * heads;
	void* container = vccalloc(container_size);
	//ExtendCcStorage(container,2 + 2 * heads);
	ExtendCcStorage(container, nodes * heads);
	ExtendPfnsStorage(container, container_size);

	HeadType** headsP;
	//NodeType** nodesP;
	NodeType* nodeP;//



	vcmalloc(container, headsP, heads);
	//vcmalloc(container, nodesP, heads);

	for (SIZE_T i = 0; i < heads; i++)
	{
		vcmalloc(container, headsP[i], 1);
		//vcmalloc(container, nodesP[i], nodes);
		new(headsP[i]) HeadType();

		for (SIZE_T j = 0; j < nodes; j++) {
			vcmalloc(container, nodeP, 1);//
			headsP[i]->perTypeAppend(i * nodes + j, nodeP);
		}
	}

	for (SIZE_T i = 0; i < heads; i++)
	{
		if (headsP[i]->search(heads * nodes + 1))
			cout << "ERROR!!!";
	}

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Allocation + Linear First Access", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);


	//Second access

	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
	{
		if (headsP[i]->search(heads * nodes + 1))
			cout << "ERROR!!!";
	}

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Linear Second Access", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);

	//free
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	vccfree(container);

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "free", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);


	return result;

}
template<typename HeadType, typename NodeType> Json::Value malloc_dispersed(SIZE_T allocSize) {
	allocSize = pow(2, allocSize);

	SIZE_T heads = sqrt(allocSize) / sqrt(sizeof(HeadType));
	SIZE_T nodes = ((allocSize / heads) - sizeof(HeadType)) / sizeof(NodeType);

	cout << "Heads: " << heads << endl;
	cout << "Nodess: " << nodes << endl;

	Json::Value result;
	result["allocator"] = "m";
	result["scenario"] = "Linked Data Structures";
	result["datatype"] = typeid(HeadType).name();
	result["size"] = allocSize;


	//evaluation variables
	clock_t c_before, c_after;						//clock
	SystemCounterState pcm_before, pcm_after;		//intel pcm
	PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status


	// allocation + first acess

	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	HeadType** headsP = (HeadType**)malloc(sizeof(HeadType*) * heads);
	if (headsP == nullptr)
		system("pause");

	for (SIZE_T i = 0; i < heads; i++)
	{
		headsP[i] = (HeadType*)malloc(sizeof(HeadType));

		if (headsP[i] == nullptr)
			system("pause");


		new(headsP[i]) HeadType();

		for (SIZE_T j = 0; j < nodes; j++)
		{
			NodeType* olln = (NodeType*)malloc(sizeof(NodeType));
			if (olln == nullptr)
				system("pause");
			headsP[i]->perTypeAppend(i * nodes + j, olln);
		}
	}

	for (SIZE_T i = 0; i < heads; i++)
	{
		if (headsP[i]->search(heads * nodes + 1))
			cout << "ERROR!!!";
	}

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Allocation + Linear First Access", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);


	//second access

	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
	{
		if (headsP[i]->search(heads * nodes + 1))
			cout << "ERROR!!!";
	}

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Linear Second Access", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);

	//free
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();


	for (size_t i = 0; i < heads; i++) {
		NodeType* to_delete = (NodeType*)headsP[i]->returnFirst();
		while (to_delete) {
			NodeType* temp = *(NodeType**)(((int*)to_delete) + 2);
			free(to_delete);
			to_delete = temp;
		}
		free(headsP[i]);
	}
	free(headsP);

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "free", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);

	return result;

}
template<typename HeadType, typename NodeType> Json::Value mi_malloc_dispersed(SIZE_T allocSize) {
	allocSize = pow(2, allocSize);

	SIZE_T heads = sqrt(allocSize) / sqrt(sizeof(HeadType));
	SIZE_T nodes = ((allocSize / heads) - sizeof(HeadType)) / sizeof(NodeType);

	cout << "Heads: " << heads << endl;
	cout << "Nodess: " << nodes << endl;

	Json::Value result;
	result["allocator"] = "mim";
	result["scenario"] = "Linked Data Structures";
	result["datatype"] = typeid(HeadType).name();
	result["size"] = allocSize;


	//evaluation variables
	clock_t c_before, c_after;						//clock
	SystemCounterState pcm_before, pcm_after;		//intel pcm
	PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status


	// allocation + first acess

	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	HeadType** headsP = (HeadType**)mi_malloc(sizeof(HeadType*) * heads);
	if (headsP == nullptr)
		system("pause");

	for (SIZE_T i = 0; i < heads; i++)
	{
		headsP[i] = (HeadType*)mi_malloc(sizeof(HeadType));

		if (headsP[i] == nullptr)
			system("pause");


		new(headsP[i]) HeadType();

		for (SIZE_T j = 0; j < nodes; j++)
		{
			NodeType* olln = (NodeType*)mi_malloc(sizeof(NodeType));
			if (olln == nullptr)
				system("pause");
			headsP[i]->perTypeAppend(i * nodes + j, olln);
		}
	}

	for (SIZE_T i = 0; i < heads; i++)
	{
		if (headsP[i]->search(heads * nodes + 1))
			cout << "ERROR!!!";
	}

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Allocation + Linear First Access", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);


	//second access

	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
	{
		if (headsP[i]->search(heads * nodes + 1))
			cout << "ERROR!!!";
	}

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Linear Second Access", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);

	//free
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();


	for (size_t i = 0; i < heads; i++) {
		NodeType* to_delete = (NodeType*)headsP[i]->returnFirst();
		int j = 0;
		while (to_delete) {
			NodeType* temp = *(NodeType**)(((int*)to_delete) + 2);
			mi_free(to_delete);
			to_delete = temp;
		}
		mi_free(headsP[i]);
	}
	mi_free(headsP);

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "free", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);

	return result;

}

template<typename Type> Json::Value vcmalloc_2d_matrix(SIZE_T allocSize)
{
	allocSize = pow(2, allocSize);
	//allocSize = (SIZE_T)8*1024*1024*1024;
	SIZE_T heads = (sqrt(allocSize) / sqrt(sizeof(Type)));
	SIZE_T nodes = ((allocSize / heads) - sizeof(Type)) / sizeof(Type);

	cout << "Number of heads: " << heads << endl;
	cout << "Number of nodes: " << nodes << endl;

	Json::Value result;
	result["allocator"] = "vcm";
	result["scenario"] = "2D Matrix";
	result["datatype"] = typeid(Type).name();
	result["size"] = allocSize;

	//evaluation variables
	clock_t c_before, c_after;						//clock
	SystemCounterState pcm_before, pcm_after;		//intel pcm
	PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status

	// allocation
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	//SIZE_T container_size = (SIZE_T)heads * nodes * sizeof(Type) + sizeof(void*) * heads;
	SIZE_T container_size = (SIZE_T)20 * 1024 * 1024 * 1024;
	void* container = vccalloc(container_size);
	ExtendCcStorage(container, heads + 1);
	ExtendPfnsStorage(container, container_size);


	Type** headsP = nullptr;

	vcmalloc(container, headsP, heads);

	for (SIZE_T i = 0; i < heads; i++)
		vcmalloc(container, headsP[i], nodes / 2);


	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Allocation", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);



	//Linear first access
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
		for (SIZE_T j = 0; j < nodes / 2; j++)
			headsP[i][j] = i * (nodes / 2) + j;

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "First Access, Linear", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);


	//Linear second access
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
		for (SIZE_T j = 0; j < nodes / 2; j++)
			headsP[i][j] = i * (nodes / 2) + j;

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Second Access, Linear", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);


	//Realloc
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();


	SIZE_T* sizes = (SIZE_T*)malloc(sizeof(SIZE_T) * heads);
	for (SIZE_T i = 0; i < heads; i++)
		sizes[i] = nodes;

	vcrealloc2(container, headsP[0], sizes, heads);

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Realloc", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);



	//Access after realloc
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
		for (SIZE_T j = 0; j < nodes; j++)
			headsP[i][j] = i * nodes + j;

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "First Access After Realloc, Linear", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);


	//Second access after realloc
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
		for (SIZE_T j = 0; j < nodes; j++)
			headsP[i][j] = i * nodes + j;

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Second Access After Realloc, Linear", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);

	//free
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	vccfree(container);

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "free", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);


	return result;

}
template<typename Type> Json::Value vcmalloc_2d_matrix_affine_array(SIZE_T allocSize)
{
	allocSize = pow(2, allocSize);
	//allocSize = (SIZE_T)8*1024*1024*1024;
	SIZE_T heads = (sqrt(allocSize) / sqrt(sizeof(Type)));
	SIZE_T nodes = ((allocSize / heads) - sizeof(Type)) / sizeof(Type);

	cout << "Number of heads: " << heads << endl;
	cout << "Number of nodes: " << nodes << endl;

	Json::Value result;
	result["allocator"] = "vcm_affarr";
	result["scenario"] = "2D Matrix";
	result["datatype"] = typeid(Type).name();
	result["size"] = allocSize;

	//evaluation variables
	clock_t c_before, c_after;						//clock
	SystemCounterState pcm_before, pcm_after;		//intel pcm
	PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status

	// allocation
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	//SIZE_T container_size = (SIZE_T)heads * nodes * sizeof(Type) + sizeof(void*) * heads;
	SIZE_T container_size = (SIZE_T)20 * 1024 * 1024 * 1024;
	void* container = vccalloc(container_size);
	ExtendCcStorage(container, heads + 1);
	ExtendPfnsStorage(container, container_size);


	Type** headsP = nullptr;

	vcmalloc(container, headsP, heads);

	for (SIZE_T i = 0; i < heads; i++)
		vcmalloc(container, headsP[i], nodes / 2);


	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Allocation", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);



	//Linear first access
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	Type* first = &headsP[0][0];
	for (SIZE_T i = 0; i < heads * (nodes / 2); i++)
		*(first++) = i;

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "First Access, Linear", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);


	//Linear second access
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	first = &headsP[0][0];
	for (SIZE_T i = 0; i < heads * (nodes / 2); i++)
		*(first++) = i;

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Second Access, Linear", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);

	return result;

}
template<typename Type> Json::Value malloc_2d_matrix(SIZE_T allocSize)
{
	allocSize = pow(2, allocSize);
	//allocSize = (SIZE_T)8 * 1024 * 1024 * 1024;
	SIZE_T heads = (sqrt(allocSize) / sqrt(sizeof(Type)));
	SIZE_T nodes = ((allocSize / heads) - sizeof(Type)) / sizeof(Type);

	cout << "Number of heads: " << heads << endl;
	cout << "Number of nodes: " << nodes << endl;

	Json::Value result;
	result["allocator"] = "m";
	result["scenario"] = "2D Matrix";
	result["datatype"] = typeid(Type).name();
	result["size"] = allocSize;

	//evaluation variables
	clock_t c_before, c_after;						//clock
	SystemCounterState pcm_before, pcm_after;		//intel pcm
	PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status


	// allocation
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	Type** headsP = (Type**)malloc(sizeof(Type*) * heads);

	for (SIZE_T i = 0; i < heads; i++) {
		headsP[i] = (Type*)malloc(sizeof(Type) * (nodes / 2));

	}

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Allocation", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);




	//Linear first access
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
		for (SIZE_T j = 0; j < nodes / 2; j++)
			headsP[i][j] = i * (nodes / 2) + j;

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "First Access, Linear", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);


	//Linear second access
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();


	for (SIZE_T i = 0; i < heads; i++)
		for (SIZE_T j = 0; j < nodes / 2; j++)
			headsP[i][j] = i * (nodes / 2) + j;

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Second Access, Linear", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);


	//Realloc
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
		headsP[i] = (Type*)realloc(headsP[i], sizeof(Type) * nodes);

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Realloc", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);



	//Access after realloc
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
		for (SIZE_T j = 0; j < nodes; j++)
			headsP[i][j] = i * (nodes)+j;

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "First Access After Realloc, Linear", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);




	//Second access after realloc
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
		for (SIZE_T j = 0; j < nodes; j++)
			headsP[i][j] = i * (nodes)+j;

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Second Access After Realloc, Linear", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);


	//free
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
		free(headsP[i]);
	free(headsP);

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "free", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);


	return result;

}
template<typename Type> Json::Value mi_malloc_2d_matrix(SIZE_T allocSize)
{
	allocSize = pow(2, allocSize);
	//allocSize = (SIZE_T)8 * 1024 * 1024 * 1024;
	SIZE_T heads = (sqrt(allocSize) / sqrt(sizeof(Type)));
	SIZE_T nodes = ((allocSize / heads) - sizeof(Type)) / sizeof(Type);

	cout << "Number of heads: " << heads << endl;
	cout << "Number of nodes: " << nodes << endl;

	Json::Value result;
	result["allocator"] = "mim";
	result["scenario"] = "2D Matrix";
	result["datatype"] = typeid(Type).name();
	result["size"] = allocSize;

	//evaluation variables
	clock_t c_before, c_after;						//clock
	SystemCounterState pcm_before, pcm_after;		//intel pcm
	PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status


	// allocation
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	Type** headsP = (Type**)mi_malloc(sizeof(Type*) * heads);

	for (SIZE_T i = 0; i < heads; i++) {
		headsP[i] = (Type*)mi_malloc(sizeof(Type) * (nodes / 2));

	}

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Allocation", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);




	//Linear first access
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
		for (SIZE_T j = 0; j < nodes / 2; j++)
			headsP[i][j] = i * (nodes / 2) + j;

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "First Access, Linear", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);


	//Linear second access
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();


	for (SIZE_T i = 0; i < heads; i++)
		for (SIZE_T j = 0; j < nodes / 2; j++)
			headsP[i][j] = i * (nodes / 2) + j;

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Second Access, Linear", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);


	//Realloc
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
		headsP[i] = (Type*)mi_realloc(headsP[i], sizeof(Type) * nodes);

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Realloc", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);



	//Access after realloc
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
		for (SIZE_T j = 0; j < nodes; j++)
			headsP[i][j] = i * (nodes)+j;

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "First Access After Realloc, Linear", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);




	//Second access after realloc
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
		for (SIZE_T j = 0; j < nodes; j++)
			headsP[i][j] = i * (nodes)+j;

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Second Access After Realloc, Linear", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);

	//free
	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	for (SIZE_T i = 0; i < heads; i++)
		mi_free(headsP[i]);
	mi_free(headsP);

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "free", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);



	return result;

}

template<typename Type> Json::Value malloc_matmult(SIZE_T allocSize) {

	//allocSize = pow(2, allocSize);
	//SIZE_T N = sqrt(allocSize) / sqrt(sizeof(Type));
	SIZE_T N = allocSize * 100;


	Json::Value result;
	result["allocator"] = "m";
	result["scenario"] = "Matrix Multiplication";
	result["datatype"] = typeid(Type).name();
	result["size"] = 100 * allocSize;

	//evaluation variables
	clock_t c_before, c_after;						//clock
	SystemCounterState pcm_before, pcm_after;		//intel pcm
	PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status

	Type** P_1 = (Type**)malloc(sizeof(Type*) * N);
	Type** P_2 = (Type**)malloc(sizeof(Type*) * N);
	Type** P_3 = (Type**)malloc(sizeof(Type*) * N);

	for (SIZE_T i = 0; i < N; i++) {
		P_1[i] = (Type*)malloc(sizeof(Type) * N);
		P_2[i] = (Type*)malloc(sizeof(Type) * N);
		P_3[i] = (Type*)malloc(sizeof(Type) * N);
	}

	for (SIZE_T i = 0; i < N; i++)
		for (SIZE_T j = 0; j < N; j++)
		{
			P_1[i][j] = i * N + j;
			P_2[i][j] = i * N + j;
			P_3[i][j] = 0;

		}

	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	Type temp = 0;
	for (SIZE_T i = 0; i < N; ++i) {
		for (SIZE_T j = 0; j < N; ++j) {
			for (SIZE_T k = 0; k < N; ++k)
			{
				temp += P_1[i][k] * P_2[k][j];
			}
			P_3[i][j] = temp;
			temp = 0;
		}
	}
	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Multiplication", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);

	return result;
}
template<typename Type> Json::Value mi_malloc_matmult(SIZE_T allocSize) {


	//allocSize = pow(2, allocSize);
	//SIZE_T N = sqrt(allocSize) / sqrt(sizeof(Type));
	SIZE_T N = allocSize * 100;

	Json::Value result;
	result["allocator"] = "mim";
	result["scenario"] = "Matrix Multiplication";
	result["datatype"] = typeid(Type).name();
	result["size"] = allocSize;

	//evaluation variables
	clock_t c_before, c_after;						//clock
	SystemCounterState pcm_before, pcm_after;		//intel pcm
	PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status


	Type** P_1 = (Type**)mi_malloc(sizeof(Type*) * N);
	Type** P_2 = (Type**)mi_malloc(sizeof(Type*) * N);
	Type** P_3 = (Type**)mi_malloc(sizeof(Type*) * N);

	for (SIZE_T i = 0; i < N; i++) {
		P_1[i] = (Type*)mi_malloc(sizeof(Type) * N);
		P_2[i] = (Type*)mi_malloc(sizeof(Type) * N);
		P_3[i] = (Type*)mi_malloc(sizeof(Type) * N);
	}

	for (SIZE_T i = 0; i < N; i++)
		for (SIZE_T j = 0; j < N; j++)
		{
			P_1[i][j] = i * N + j;
			P_2[i][j] = i * N + j;
			P_3[i][j] = 0;

		}

	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	Type temp = 0;
	for (SIZE_T i = 0; i < N; ++i) {
		for (SIZE_T j = 0; j < N; ++j) {
			for (SIZE_T k = 0; k < N; ++k)
			{
				temp += P_1[i][k] * P_2[k][j];
			}
			P_3[i][j] = temp;
			temp = 0;
		}
	}
	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Multiplication", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);

	return result;
}

template<typename Type> Json::Value vcmalloc_matmult(SIZE_T allocSize) {


	//allocSize = pow(2, allocSize);
	//SIZE_T N = sqrt(allocSize) / sqrt(sizeof(Type));
	SIZE_T N = allocSize * 100;

	Json::Value result;
	result["allocator"] = "vcm";
	result["scenario"] = "Matrix Multiplication";
	result["datatype"] = typeid(Type).name();
	result["size"] = allocSize;

	//evaluation variables
	clock_t c_before, c_after;						//clock
	SystemCounterState pcm_before, pcm_after;		//intel pcm
	PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status



	void* P_container = (void*)vccalloc((SIZE_T)16 * 1024 * 1024 * 1024);
	void* container = (void*)vccalloc((SIZE_T)16 * 1024 * 1024 * 1024);

	Type** P_1, ** P_2, ** P_3;
	vcmalloc(P_container, P_1, N);
	vcmalloc(P_container, P_2, N);
	vcmalloc(P_container, P_3, N);

	for (SIZE_T i = 0; i < N; i++)
		vcmalloc(container, P_1[i], N);
	for (SIZE_T i = 0; i < N; i++)
		vcmalloc(container, P_2[i], N);
	for (SIZE_T i = 0; i < N; i++)
		vcmalloc(container, P_3[i], N);

	for (SIZE_T i = 0; i < N; i++)
		for (SIZE_T j = 0; j < N; j++)
		{
			P_1[i][j] = i * N + j;
			P_2[i][j] = i * N + j;
			P_3[i][j] = 0;

		}

	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	Type temp = 0;
	for (SIZE_T i = 0; i < N; ++i) {
		for (SIZE_T j = 0; j < N; ++j) {
			for (SIZE_T k = 0; k < N; ++k)
			{
				temp += P_1[i][k] * P_2[k][j];
			}
			P_3[i][j] = temp;
			temp = 0;
		}
	}

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Multiplication", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);

	return result;
}
template<typename Type> Json::Value vcmalloc_matmult_single(SIZE_T allocSize) {


	//allocSize = pow(2, allocSize);
	//SIZE_T N = sqrt(allocSize) / sqrt(sizeof(Type));
	SIZE_T N = allocSize * 100;

	Json::Value result;
	result["allocator"] = "vcm_s";
	result["scenario"] = "Matrix Multiplication";
	result["datatype"] = typeid(Type).name();
	result["size"] = allocSize;

	//evaluation variables
	clock_t c_before, c_after;						//clock
	SystemCounterState pcm_before, pcm_after;		//intel pcm
	PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status



	void* container = (void*)vccalloc((SIZE_T)16 * 1024 * 1024 * 1024);

	Type** P_1, ** P_2, ** P_3;

	vcmalloc(container, P_1, N);
	for (SIZE_T i = 0; i < N; i++)
		vcmalloc(container, P_1[i], N);
	vcmalloc(container, P_2, N);
	for (SIZE_T i = 0; i < N; i++)
		vcmalloc(container, P_2[i], N);
	vcmalloc(container, P_3, N);
	for (SIZE_T i = 0; i < N; i++)
		vcmalloc(container, P_3[i], N);

	for (SIZE_T i = 0; i < N; i++)
		for (SIZE_T j = 0; j < N; j++)
		{
			P_1[i][j] = i * N + j;
			P_2[i][j] = i * N + j;
			P_3[i][j] = 0;

		}

	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	Type temp = 0;
	for (SIZE_T i = 0; i < N; ++i) {
		for (SIZE_T j = 0; j < N; ++j) {
			for (SIZE_T k = 0; k < N; ++k)
			{
				temp += P_1[i][k] * P_2[k][j];
			}
			P_3[i][j] = temp;
			temp = 0;
		}
	}

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Multiplication", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);

	return result;
}
template<typename Type> Json::Value vcmalloc_matmult_single_pointer_first(SIZE_T allocSize) {


	//allocSize = pow(2, allocSize);
	//SIZE_T N = sqrt(allocSize) / sqrt(sizeof(Type));
	SIZE_T N = allocSize * 100;

	Json::Value result;
	result["allocator"] = "vcm_spf";
	result["scenario"] = "Matrix Multiplication";
	result["datatype"] = typeid(Type).name();
	result["size"] = allocSize;

	//evaluation variables
	clock_t c_before, c_after;						//clock
	SystemCounterState pcm_before, pcm_after;		//intel pcm
	PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status



	void* container = (void*)vccalloc((SIZE_T)16 * 1024 * 1024 * 1024);

	Type** P_1, ** P_2, ** P_3;

	vcmalloc(container, P_1, N);
	vcmalloc(container, P_2, N);
	vcmalloc(container, P_3, N);

	for (SIZE_T i = 0; i < N; i++)
		vcmalloc(container, P_1[i], N);
	for (SIZE_T i = 0; i < N; i++)
		vcmalloc(container, P_2[i], N);
	for (SIZE_T i = 0; i < N; i++)
		vcmalloc(container, P_3[i], N);

	for (SIZE_T i = 0; i < N; i++)
		for (SIZE_T j = 0; j < N; j++)
		{
			P_1[i][j] = i * N + j;
			P_2[i][j] = i * N + j;
			P_3[i][j] = 0;

		}

	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	Type temp = 0;
	for (SIZE_T i = 0; i < N; ++i) {
		for (SIZE_T j = 0; j < N; ++j) {
			for (SIZE_T k = 0; k < N; ++k)
			{
				temp += P_1[i][k] * P_2[k][j];
			}
			P_3[i][j] = temp;
			temp = 0;
		}
	}

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Multiplication", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);

	return result;
}
template<typename Type> Json::Value vcmalloc_matmult_single_affine_array(SIZE_T allocSize) {


	//allocSize = pow(2, allocSize);
	//SIZE_T N = sqrt(allocSize) / sqrt(sizeof(Type));
	SIZE_T N = allocSize * 100;

	Json::Value result;
	result["allocator"] = "vcm_affarr";
	result["scenario"] = "Matrix Multiplication";
	result["datatype"] = typeid(Type).name();
	result["size"] = allocSize;

	//evaluation variables
	clock_t c_before, c_after;						//clock
	SystemCounterState pcm_before, pcm_after;		//intel pcm
	PROCESS_MEMORY_COUNTERS ps_before, ps_after;	//windows process status



	void* container = (void*)vccalloc((SIZE_T)16 * 1024 * 1024 * 1024);

	Type** P_1, ** P_2, ** P_3;

	vcmalloc(container, P_1, N);
	vcmalloc(container, P_2, N);
	vcmalloc(container, P_3, N);

	for (SIZE_T i = 0; i < N; i++)
		vcmalloc(container, P_1[i], N);
	for (SIZE_T i = 0; i < N; i++)
		vcmalloc(container, P_2[i], N);
	for (SIZE_T i = 0; i < N; i++)
		vcmalloc(container, P_3[i], N);

	for (SIZE_T i = 0; i < N; i++)
		for (SIZE_T j = 0; j < N; j++)
		{
			P_1[i][j] = i * N + j;
			P_2[i][j] = i * N + j;
			P_3[i][j] = 0;

		}

	//before
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_before, sizeof(ps_before));
	pcm_before = getSystemCounterState();
	c_before = clock();

	auto firstElem = *P_1;
	Type temp = 0;
	for (SIZE_T i = 0; i < N; ++i) {
		for (SIZE_T j = 0; j < N; ++j) {
			for (SIZE_T k = 0; k < N; ++k)
			{
				//temp += P_1[i][k] * P_2[k][j];
				temp += firstElem[i * N + k] * firstElem[k * N + j + N * N];

			}
			//P_3[i][j] = temp;
			firstElem[i * N + j + 2 * N * N] = temp;
			temp = 0;
		}
	}

	//after
	c_after = clock();
	pcm_after = getSystemCounterState();
	GetProcessMemoryInfo(GetCurrentProcess(), &ps_after, sizeof(ps_after));
	result_filler(result, "Multiplication", c_before, c_after, pcm_before, pcm_after, ps_before, ps_after);

	return result;
}

int main(int argc, char* argv[]) {

	PCM* m = PCM::getInstance();
	m->resetPMU();
	if (m->program() != PCM::Success) return 0;

	typedef Json::Value(*tests_f)(SIZE_T allocSize);
	tests_f tests[] = {
		/*0*/	&vcmalloc_single<		unorderedLinkedList<double>,		linkedListNodeType<double>>,
		/*1*/	&vcmalloc_single<		stackUnorderedLinkedList<double>,	linkedListNodeType<double>>,
		/*2*/	&vcmalloc_single<		linkedQueueType<double>,			linkedQueuesNodeType<double>>,
		/*3*/	&vcmalloc_single<		linkedStackType<double>,			linkedStackNodeType<double>>,
		/*4*/	&vcmalloc_single<		orderedLinkedList<double>,			linkedListNodeType<double>>,
		/*5*/	&vcmalloc_single<		doublyLinkedList<double>,			doublyLinkedListNodeType<double>>,
		/*6*/	&vcmalloc_single<		bSearchTreeType<double>,			binaryTreeNodeType<double>>,

		/*7*/	&malloc_dispersed<		unorderedLinkedList<double>,		linkedListNodeType<double>>,
		/*8*/	&malloc_dispersed<		stackUnorderedLinkedList<double>,	linkedListNodeType<double>>,
		/*9*/	&malloc_dispersed<		linkedQueueType<double>,			linkedQueuesNodeType<double>>,
		/*10*/	&malloc_dispersed<		linkedStackType<double>,			linkedStackNodeType<double>>,
		/*11*/	&malloc_dispersed<		orderedLinkedList<double>,			linkedListNodeType<double>>,
		/*12*/	&malloc_dispersed<		doublyLinkedList<double>,			doublyLinkedListNodeType<double>>,
		/*13*/	&malloc_dispersed<		bSearchTreeType<double>,			binaryTreeNodeType<double>>,

		/*14*/	&mi_malloc_dispersed<	unorderedLinkedList<double>,		linkedListNodeType<double>>,
		/*15*/	&mi_malloc_dispersed<	stackUnorderedLinkedList<double>,	linkedListNodeType<double>>,
		/*16*/	&mi_malloc_dispersed<	linkedQueueType<double>,			linkedQueuesNodeType<double>>,
		/*17*/	&mi_malloc_dispersed<	linkedStackType<double>,			linkedStackNodeType<double>>,
		/*18*/	&mi_malloc_dispersed<	orderedLinkedList<double>,			linkedListNodeType<double>>,
		/*19*/	&mi_malloc_dispersed<	doublyLinkedList<double>,			doublyLinkedListNodeType<double>>,
		/*20*/	&mi_malloc_dispersed<	bSearchTreeType<double>,			binaryTreeNodeType<double>>,

		/*21*/	&vcmalloc_2d_matrix<double>,
		/*22*/	&vcmalloc_2d_matrix_affine_array<double>,
		/*23*/	&malloc_2d_matrix<double>,
		/*24*/	&mi_malloc_2d_matrix<double>,

		/*25*/	&malloc_matmult<double>,
		/*26*/	&mi_malloc_matmult<double>,

		/*27*/	&vcmalloc_matmult_single_affine_array<double>,
		/*28*/	&vcmalloc_matmult_single_pointer_first<double>,
		/*29*/	&vcmalloc_matmult<double>,
		/*30*/	&vcmalloc_matmult_single<double>,

	};

	int test_id = stoi(argv[1]);
	int size_choice = stoi(argv[2]);
	int nbr_of_test = stoi(argv[3]);


	//SIZE_T size = pow(2, size_choice);

	Json::Value n_rslt = tests[test_id](size_choice);

	std::string fileName = "results.json";

	to_json(n_rslt, nbr_of_test, fileName);
	if (argc == 5) to_json(n_rslt, nbr_of_test, argv[4]);

	m->cleanup();

	return 0;
}

