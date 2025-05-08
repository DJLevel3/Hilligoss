cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
project(HilligossProject CXX)

add_library(Hilligoss   src/hilligoss.cpp
                        src/hilligoss.h)

add_executable(hilligoss-nodeps src/main-nodeps.cpp)
target_link_libraries(hilligoss-nodeps Hilligoss)

find_package(OpenCV) 

if (DEFINED OpenCV_FOUND)
    include_directories(include ${OpenCV_INCLUDE_DIRS} src)
    add_executable(Hilligoss-OpenCV src/main-opencv.cpp)
    target_link_libraries(Hilligoss-OpenCV ${OpenCV_LIBS} Hilligoss)
endif()