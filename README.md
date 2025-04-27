# Hilligoss!
Now with full cross-platform (Windows, MacOS, Linux) support!

### Requirements:
1. OpenCV (with all environtment variables set up)
2. Modern C++ compiler with C++23 standard support(MinGW, MSVC, Clang).
3. Build automation program which is able to handle generated make file by CMake
	(should be added to compiler's package and can be run with terminal or using IDE).
4. CMake at least version 3.10.

### Getting OpenCV:
- Windows (MinGW): `pacman -S mingw-w64-x86_64-opencv`
- Linux (Debian-like): `sudo apt install libopencv-dev`
- Linux (Arch-like): `pacman -S opencv`
- MacOS: `brew install opencv`
- Windows (MSVC, not recommended due to being unbearably slow): Download package from https://github.com/opencv/opencv/releases/latest

### Building:
Note: built openCV library must be already installed and accessible to CMake
```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```
