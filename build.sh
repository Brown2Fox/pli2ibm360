#!/bin/bash

mkdir cmake_build
mkdir build

cd cmake_build
cmake -DBUILD_DIR=cmake_build ..
make

mv asm2obj ../build/
mv ibm360vm ../build/
mv pli2asm ../build/