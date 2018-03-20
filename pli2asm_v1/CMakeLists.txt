
cmake_minimum_required(VERSION 3.4)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-everything")

set(PLI2ASM_V1_SOURCES pli2asm_v1.c)
add_executable(pli2asm_v1 ${PLI2ASM_V1_SOURCES})