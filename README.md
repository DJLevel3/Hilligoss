# Hilligoss
A pixel-to-vector tool and library by BUS ERROR Collective

### Usage
`Hilligoss-2.0 -i "your-video-here.mp4" [options]` (use -h to see the full list of options)

# Building - READ EVERYTHING!

### Requirements:
1. OpenCV binaries and development libraries
2. Ncurses
3. GCC (using MinGW on Windows or native on MacOS or Linux)
4. CMake >=3.10
5. (Windows) Qt 6.9.0

### Getting dependencies and toolchain
- Windows (MinGW): `pacman -S --needed mingw-w64-x86_64-cmake mingw-w64-x86_64-make mingw-w64-x86_64-gcc mingw-w64-x86_64-ncurses mingw-w64-x86_64-opencv mingw-w64-x86_64-qt6-5compat`
- Linux (Debian-like): `sudo apt install git cmake make gcc libncurses-dev libopencv-dev`
- Linux (Arch-like): `pacman -S --needed git cmake make gcc ncurses opencv`
- MacOS: see below

### MacOS toolchain setup
1. Install xcode command line developer tools: `sudo xcode-select --install`
2. Install [MacPorts](https://www.macports.org/install.php)
3. Install dependencies: `sudo port -N install cmake ncurses opencv4`

If the build fails with the above dependencies, you may need GCC as well: `sudo port -N install gcc15` (Beware, the download is quite large!)

### CRITICAL - Windows notes
- DO NOT use Visual Studio or cmake GUI for this! It will not compile without LOTS of work, and the end result will be as much as 10x slower to process each frame! Instead use MinGW, which you can get [here](https://msys2.org) (note that this doesn't include Git, so you'll need to either install that or download the zip instead
- When running the commands, run them in a `MSYS2 MinGW x64` shell.
- Install the required dependencies as shown above, and install them EXACTLY as written. I say this, because if you use `gcc` instead of `mingw-w64-x86_64-gcc`, Hilligoss will crash when it renders frames.
- Make sure your MSYS MinGW bin folder is on PATH (usually located at `C:\msys64\mingw64\bin`)
- If you get error popups including  `The code execution cannot proceed because Qt6Core.dll was not found. Reinstalling the program may fix this problem.` when launching, make sure your MinGW dependencies (especially `mingw-w64-x86_64-qt6-5compat`) are installed and on PATH!
	
### Building (make):
```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

### Building (ninja):
Note: you may need to do this on Windows. You'll know to use this one if it says `-- Building For: Ninja` during the cmake step.
```
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
ninja
```