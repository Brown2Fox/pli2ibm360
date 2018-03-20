#!/bin/bash

BUILD_DIR=".build"
CMAKE_DIR=".cmake_build"

[ -d $CMAKE_DIR ] || mkdir $CMAKE_DIR
[ -d $BUILD_DIR ] || mkdir $BUILD_DIR

cd $CMAKE_DIR
cmake -DBUILD_DIR=$CMAKE_DIR ..
make $@

[ -f asm2obj ] && mv asm2obj ../$BUILD_DIR/
[ -f ibm360vm ] && mv ibm360vm ../$BUILD_DIR/
[ -f pli2asm ] && mv pli2asm ../$BUILD_DIR/
[ -f pli2asm_v1 ] && mv pli2asm_v1 ../$BUILD_DIR/