@echo off & CHCP 65001
@title build project

cd ..

echo "create build"
if not exist build (
    mkdir build
)

echo "create bin"
if not exist bin (
    mkdir bin
)

cd build

conan install .. -s build_type=Debug
cmake ..
cmake --build . --config Release

pause