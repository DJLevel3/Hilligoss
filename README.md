# Hilligoss
A pixel-to-vector tool and library by BUS ERROR Collective

### Usage
```
Hilligoss-OpenCV -i "your-video-here.mp4"
Hilligoss-OpenCV -i -black 30 -white 255 -jump 500 -rate 192000 -distance 40 -framerate 24 -curve 1 -threads 15 -frameloop 3 -boost 5
```

### Requirements:
1. OpenCV binaries and development library
2. Ncurses
3. GCC (using MinGW on Windows or native on MacOS or Linux)
4. CMake >=3.10
5. (Windows) Qt 6.9.0

### Getting OpenCV:
- Windows (MinGW): `pacman -S mingw-w64-x86_64-opencv`
- Linux (Debian-like): `sudo apt install libopencv-dev`
- Linux (Arch-like): `pacman -S opencv`
- MacOS: `brew install opencv`

### Building:
```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

### Windows - MSYS2/MinGW Setup
1. Get and install MSYS2 [here](https://msys2.org)
2. Launch `MSYS2 MinGW x64`
3. Install required packages using `pacman -S mingw-w64-x86_64-make mingw-w64-x86_64-cmake cmake mingw-w64-x86_64-gcc mingw-w64-x86_64-opencv mingw-w64-x86_64-qt6-5compat mingw-w64-x86_64-ncurses`
    - CRITICAL - Make sure to use `mingw-w64-x86_64-gcc` and NOT `gcc`, or it will crash when processing frames! It took a 3-hour debugging session to figure this out!

### Notes (Windows):
- MinGW/MSYS2 is the recommended toolchain for Windows. Get the installer [here](https://www.msys2.org/)
- OpenCV binaries and headers must be installed and on PATH. Install through MinGW as mentioned above.
- Qt 6 MinGW binaries must be installed and on PATH. Get the installer [here](https://www.qt.io/download-qt-installer-oss).
- If you get error popups including  `The code execution cannot proceed because Qt6Core.dll was not found. Reinstalling the program may fix this problem.` when launching, make sure Qt6 is installed and on PATH. 
- MSVC and will *technically* work with lots of effort, but it's more frustrating and the end product is slower. I (DJ_Level_3) personally develop in Visual Studio 2022, but I use MinGW/GCC to compile it.
