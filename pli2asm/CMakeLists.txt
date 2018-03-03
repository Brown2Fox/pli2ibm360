
cmake_minimum_required(VERSION 3.4)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-everything")

set(MSYS_DIR E:/msys64)

set(BISON_EXECUTABLE ${MSYS_DIR}/usr/bin/bison.exe)
find_package(BISON REQUIRED)
set(FLEX_EXECUTABLE ${MSYS_DIR}/usr/bin/flex.exe)
find_package(FLEX REQUIRED)
BISON_TARGET(PARSER pli2asm.y ${CMAKE_CURRENT_BINARY_DIR}/pli2asm.parser.c)
FLEX_TARGET(LEXER pli2asm.l ${CMAKE_CURRENT_BINARY_DIR}/pli2asm.lexer.c)
ADD_FLEX_BISON_DEPENDENCY(LEXER PARSER)

add_executable(pli2asm ${BISON_PARSER_OUTPUTS} ${FLEX_LEXER_OUTPUTS})