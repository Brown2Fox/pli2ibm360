
cmake_minimum_required(VERSION 3.4)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-everything")

set(IBM360VM_SOURCES ibm360vm.c)
add_executable(ibm360vm ${IBM360VM_SOURCES})
target_link_libraries(ibm360vm ncurses)