cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_COMPILER g++)
project(HilligossProject CXX)

include_directories(include src)
add_library(Hilligoss src/hilligoss.cpp)

add_executable(hilligoss-nodeps src/main-nodeps.cpp)
target_link_libraries(hilligoss-nodeps Hilligoss)


set(CURSES_NEED_NCURSES TRUE)
find_package(Curses)

if (DEFINED CURSES_FOUND)
find_package(OpenCV COMPONENTS core highgui videoio imgproc) 
if (DEFINED OpenCV_FOUND)
    set(CMAKE_CXX_FLAGS "-DNCURSES_STATIC")
    add_executable(Hilligoss-OpenCV src/main-opencv.cpp)
    target_include_directories(Hilligoss-OpenCV PUBLIC ${CURSES_INCLUDE_DIRS} include src ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(Hilligoss-OpenCV ncurses -static Hilligoss ${OpenCV_LIBS})
endif()
endif()