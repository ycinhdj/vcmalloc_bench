# vcmalloc_bench
These are a set of programs and scripts made specifically to test the performance of VCMalloc, MIMalloc and Malloc.

The folder "libs_src" contains the libraries sources used in the tests: VCMalloc, MIMalloc and Intel PCM
The folder "libs_static" contains the built libraries of MIMalloc and Intel PCM (The build instructions for each library are available in their respective repositories)
The folders "vcmalloc\_bench" and "vcmalloc\_bench_cuda" contain the source code of the test programs
the folder "out" contains the build output of the test programs
The folder "scripts" contains batch and python scripts to run successive tests on different scenarios and the python program reads the trsts results file (.json) and plot those results into figures

To build these test programs:
requirements:
Visual Studio 2019 with VC++ latest version
vcpkg with jsoncpp (x64) and visual studio integration
python with the libraries:
pandas, matplotlib, numpy

Steps:
add lock pages in memory priviledge to the logged user
clone this repository
open .sln file and build
run the tests

To run the tests:
cd into "scripts" folder with an elevated terminal
run .cmd file of the desired scenario
after the tests are finished run read\_n\_plot.py to generate figures