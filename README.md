# vcmalloc_bench
These are a set of programs and scripts made specifically to test the performance and pertinence of [VCMalloc](https://github.com/ycnhdj/vcmalloc), [MIMalloc](https://github.com/microsoft/mimalloc) and Malloc.
## Project structure
- The folder [libs_src](libs_src) contains the libraries sources used in the tests: [VCMalloc](https://github.com/ycnhdj/vcmalloc), [MIMalloc](https://github.com/microsoft/mimalloc) and [Intel PCM](https://github.com/opcm/pcm)
- The folder [libs_static](libs_static) contains the built libraries of [MIMalloc](https://github.com/microsoft/mimalloc) and [Intel PCM](https://github.com/opcm/pcm) (The build instructions for each library are available in their respective repositories)
- The folders [vcmalloc\_bench](vcmalloc\_bench) and [vcmalloc\_bench_cuda](vcmalloc\_bench_cuda) contain the source code of the test programs
- The folder [out](out) contains the build output of the test programs
- The folder [scripts](scripts) contains batch and python scripts to run successive tests on different scenarios and the python program reads the trsts results file (.json) and plot those results into figures
## Building
### Requirements
- Visual Studio 2019 with VC++ latest version
- vcpkg with jsoncpp (x64) and visual studio integration
### Steps:
- [Add lock pages in memory priviledge](https://docs.microsoft.com/en-us/sql/database-engine/configure-windows/enable-the-lock-pages-in-memory-option-windows) to the logged user
- Clone this repository
- Open .sln file with Visual Studio 2019 and build with "Release" mode and "x64" architecture.
## Pre-Built Binaries
This repository contains already built binaries that can be directly used to launch the tests, the steps to run the pre-built binaries are:
- Clone this repository
- Launch the ".cmd" scripts in [scripts](scripts)
## Generating the figures
### Requirements
To generate figures from the tests results, python 3 is reauired with the following libraries
- pandas
- matplotlib
- numpy
### Steps
After the tests are finished run read\_n\_plot.py located inside [scripts](scripts) folder to generate figures
## Acknowledgment:
This project is initiated by [RCAM](https://www.univ-sba.dz/rcam) laboratory as part of the thesis project realized by [Yacine Hadjadj](https://www.linkedin.com/in/ycinhdj) and supervised by [Taleb Nasreddine](https://scholar.google.com/citations?user=tjpgMDAAAAAJ) and [CMA Zouaoui](https://www.univ-sba.dz/rcam/index.php/equipes/equipe-4?view=article&id=25&catid=8).
## Contact:
yacine.hadjadj@univ-sba.dz
ycin.hdj@gmail.com