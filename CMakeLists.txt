cmake_minimum_required(VERSION 3.10)
project(OpenCVProject)
set(CMAKE_CXX_STANDARD 23)

find_package(OpenCV CONFIG PATHS "%OPENCV_DIR%") 

add_library(Hilligoss   src/hilligoss.cpp
                        src/hilligoss.h)

add_executable(Hilligoss-OpenCV src/main-opencv.cpp)
add_executable(hilligoss-nodeps src/main-nodeps.cpp)

target_link_libraries(hilligoss-nodeps Hilligoss)

if (DEFINED OpenCV_FOUND)
    include_directories(include ${OpenCV_INCLUDE_DIRS} src)
    target_link_libraries(Hilligoss-OpenCV ${OpenCV_LIBS} Hilligoss)
endif()