
cmake_minimum_required(VERSION 3.4)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-everything")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-everything -std=c++11")


include_directories(Operations)
include_directories(Cards)

# for Clion's code inspection
FILE(GLOB FILES
        ./*.hpp
        Operations/*.hpp
        Operations/Pseudo/*.hpp
        Operations/Regular/*.hpp
        Cards/*.hpp)

set(ASM2OBJ_SOURCES asm2obj.cpp ${FILES})

add_executable(asm2obj ${ASM2OBJ_SOURCES})