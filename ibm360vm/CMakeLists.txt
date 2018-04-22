
cmake_minimum_required(VERSION 3.4)


set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-everything")

set(IBM360VM_SOURCES main.cpp)
add_executable(ibm360vm ${IBM360VM_SOURCES})
target_link_libraries(ibm360vm ncurses)