cmake_minimum_required(VERSION 3.10)
project(OpenCVProject)
set(CMAKE_CXX_STANDARD 23)

find_package(OpenCV CONFIG REQUIRED PATHS "%OPENCV_DIR%") 

include_directories(include ${OpenCV_INCLUDE_DIRS} src)
add_executable(Hilligoss-OpenCV src/main.cpp 
                            src/hilligoss.cpp
                            src/hilligoss.h)
target_link_libraries(Hilligoss-OpenCV ${OpenCV_LIBS})