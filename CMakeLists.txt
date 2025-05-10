cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_GENERATOR "Unix Makefiles")
project(HilligossProject CXX)


include_directories(include src )
add_library(Hilligoss   src/hilligoss.cpp
                        src/hilligoss.h)

add_executable(hilligoss-nodeps src/main-nodeps.cpp)
target_link_libraries(hilligoss-nodeps Hilligoss)


set(CURSES_NEED_NCURSES TRUE)
set(CURSES_NEED_WIDE TRUE)
find_package(Curses)
find_package(OpenCV) 

if (DEFINED CURSES_FOUND)
if (DEFINED OpenCV_FOUND)
    include_directories(include ${OpenCV_INCLUDE_DIRS})
    add_executable(Hilligoss-OpenCV src/main-opencv.cpp)
    target_link_libraries(Hilligoss-OpenCV ncursesw ${OpenCV_LIBS} ${CURSES_LIBS} Hilligoss)
endif()
endif()