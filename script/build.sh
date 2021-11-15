#!/usr/bin/env bash

test -e ../build || mkdir -p ../build
echo "mkdir build"

test -e ../bin || mkdir -p ../bin
echo "mkdir bin"

cd ../build
conan install ..
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
