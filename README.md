# vcmalloc_bench
These are a set of programs and scripts made specifically to test the performance and pertinence of [VCMalloc](https://github.com/ycnhdj/vcmalloc), [MIMalloc](https://github.com/microsoft/mimalloc) and Malloc.
## Project structure
- The folder [libs_src](libs_src) contains the sources of the libraries used in the tests: [VCMalloc](https://github.com/ycnhdj/vcmalloc), [MIMalloc](https://github.com/microsoft/mimalloc) and [Intel PCM](https://github.com/opcm/pcm)
- The folder [libs_static](libs_static) contains the built libraries of [MIMalloc](https://github.com/microsoft/mimalloc) and [Intel PCM](https://github.com/opcm/pcm) (The build instructions for each library are available in their respective repositories)
- The folders [vcmalloc\_bench](vcmalloc\_bench) and [vcmalloc\_bench_cuda](vcmalloc\_bench_cuda) contain the source code of the test programs
- The folder [out](out) contains the build output of the test programs
- The folder [scripts](scripts) contains batch and python scripts that can be used to:
    - Run successive tests on different scenarios
    - Read the tests results file (.json) and plot those results into figures
## Building
### Requirements
- [Visual Studio 2019](https://visualstudio.microsoft.com/vs/older-downloads/) with "Desktop development with C++" workload and "C++/CLI support for build tools" for the latest version in installation details
- [Vcpkg](https://github.com/microsoft/vcpkg) with [JsonCpp](https://github.com/open-source-parsers/jsoncpp) (x64) and Visual Studio integration
- [CUDA Toolkit](https://developer.nvidia.com/cuda-downloads?target_os=Windows&target_arch=x86_64)
### Steps:
- [Add lock pages in memory priviledge](https://docs.microsoft.com/en-us/sql/database-engine/configure-windows/enable-the-lock-pages-in-memory-option-windows) to the logged user
- Install [Visual Studio 2019](https://visualstudio.microsoft.com/vs/older-downloads/)
- Install [CUDA Toolkit](https://developer.nvidia.com/cuda-downloads?target_os=Windows&target_arch=x86_64)
- Install [Git](https://git-scm.com/download/win)
- Install [Vcpkg](https://github.com/microsoft/vcpkg/)
- Install [JsonCpp](https://github.com/open-source-parsers/jsoncpp/)
```console
vcpkg install jsoncpp:x64-windows
```
- Clone this repository 
```console
git clone https://github.com/ycnhdj/vcmalloc_bench.git
```
- Open .sln file with Visual Studio 2019 and build with "Release" mode and "x64" architecture.
## Pre-Built Binaries
This repository contains already built binaries that can be directly used to launch the tests, the steps to run the pre-built binaries are:
- [Add lock pages in memory priviledge](https://docs.microsoft.com/en-us/sql/database-engine/configure-windows/enable-the-lock-pages-in-memory-option-windows) to the logged user
- Clone this repository
```console
git clone https://github.com/ycnhdj/vcmalloc_bench.git
```
- cd to [scripts](scripts) folder in an elevated command prompt
- Launch the ".cmd" files
## Generating the figures
This repository has the results from the tests that we performed (.json files in [scripts](scripts) folder), with their generated figures.
### Requirements
To generate figures from the tests results, python 3 is required with the following libraries
- pandas
- matplotlib
- numpy
### Steps
- Install [Python](https://www.python.org/downloads/)
- Install required python libraries
```console
pip install matplotlib numpy pandas
```
- Run read\_n\_plot.py
```console
py read_and_plot.py
```
## Acknowledgment:
This project is initiated by [RCAM](https://www.univ-sba.dz/rcam) laboratory as part of the thesis project realized by [Yacine Hadjadj](https://www.linkedin.com/in/ycinhdj) and supervised by [Taleb Nasreddine](https://scholar.google.com/citations?user=tjpgMDAAAAAJ) and [CMA Zouaoui](https://scholar.google.com/citations?user=LWPvZI4AAAAJ).
## Contact:
yacine.hadjadj@univ-sba.dz  
mail@ycinhdj.com
