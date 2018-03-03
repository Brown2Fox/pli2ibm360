
cmake_minimum_required(VERSION 3.4)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-everything")

set(ASM2OBJ_SOURCES asm2obj.c)
add_executable(asm2obj ${ASM2OBJ_SOURCES})