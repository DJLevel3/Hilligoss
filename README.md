# Quick setup for project using CMake and OpenCV.

### Requirements:
1. OpenCV
   - must be built with the same compiler which will be used in project,
   - under Windows all environment variables must be set correctly
     (i.e. OpenCV_DIR created and bin folder with all dlls added do PATH).
2. Modern C++ compiler with C++23 standard support(MinGW, MSVC, Clang).
3. Build automation program which is able to handle generated make file by CMake
	(should be added to compiler's package and can be run with terminal or using IDE).
4. CMake at least version 3.10.

### Building:
Note: built openCV library must be already installed and accessible to CMake
```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```
