# vcmalloc_bench
These are a set of programs and scripts made specifically to test the performance and pertinence of VCMalloc, MIMalloc and Malloc.

## Project structure
-The folder "libs_src" contains the libraries sources used in the tests: VCMalloc, MIMalloc and Intel PCM
- The folder "libs_static" contains the built libraries of MIMalloc and Intel PCM (The build instructions for each library are available in their respective repositories)
- The folders "vcmalloc\_bench" and "vcmalloc\_bench_cuda" contain the source code of the test programs
- The folder "out" contains the build output of the test programs
- The folder "scripts" contains batch and python scripts to run successive tests on different scenarios and the python program reads the trsts results file (.json) and plot those results into figures

##Building
### Requirements:
- Visual Studio 2019 with VC++ latest version
- vcpkg with jsoncpp (x64) and visual studio integration
### Steps:
- Add lock pages in memory priviledge to the logged user
- Clone this repository
- Open .sln file with Visual Studio 2019 and build with "Release" mode and "x64" architecture.

## Pre-Built Binaries
This repository contains already built binaries that can be directly used to launch the tests, the steps to run the pre-built binaries are:
- Clone this repository
- Launch the ".cmd" scripts in [scripts](/scripts)

## Generating the figures
### Requirements
To generate figures from the tests results, python 3 is reauires with the following libraries
- pandas
- matplotlib
- numpy
### Steps
cd into "scripts" folder with an elevated terminal
run .cmd file of the desired scenario
after the tests are finished run read\_n\_plot.py to generate figures

## Acknowledgment:
This project is initiated by RCAM laboratory as part of the thesis project realized by Yacine Hadjadj and supervised by Taleb Nasreddine and CMA Zouaoui.

## Contact:
yacine.hadjadj@univ-sba.dz
ycin.hdj@gmail.com