
cmake_minimum_required(VERSION 3.4)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-everything -std=c++11")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-everything")

set(ASM2OBJ_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/asm2obj/asm2obj.cpp)
set(IBM360VM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/ibm360vm/main.cpp)
set(PLI2ASM_V1_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/pli2asm_v1/pli2asm_v1.c)

include_directories(include)
include_directories(asm2obj/Operations)
include_directories(asm2obj/Cards)

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
BISON_TARGET(PARSER ${CMAKE_CURRENT_SOURCE_DIR}/pli2asm/pli2asm.y ${CMAKE_CURRENT_SOURCE_DIR}/${BUILD_DIR}/pli2asm.parser.c)
FLEX_TARGET(LEXER ${CMAKE_CURRENT_SOURCE_DIR}/pli2asm/pli2asm.l ${CMAKE_CURRENT_SOURCE_DIR}/${BUILD_DIR}/pli2asm.lexer.c)
ADD_FLEX_BISON_DEPENDENCY(LEXER PARSER)

add_executable(asm2obj ${ASM2OBJ_SOURCES})
add_executable(ibm360vm ${IBM360VM_SOURCES})
add_executable(pli2asm ${BISON_PARSER_OUTPUTS} ${FLEX_LEXER_OUTPUTS})
add_executable(pli2asm_v1 ${PLI2ASM_V1_SOURCES})

target_link_libraries(ibm360vm ncurses)