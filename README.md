# Hilligoss!
Now with full cross-platform (Windows, MacOS, Linux) support!

### Requirements:
1. OpenCV (with all environtment variables set up)
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
