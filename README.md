# PISANO

PISANO, that's **P**redicate-**I**ndexed **S**ubject **AN**d **O**bject

![](https://img.shields.io/badge/Conan-1.42-brightgreen)

## build

Make sure you have installed [conan](https://conan.io/), and can use it correctly to build with [CMake](https://cmake.org/)

If you're not familiar with conan, you can install it by pip, that is `pip install conan`, and have a [quick start](https://docs.conan.io/en/latest/getting_started.html)

> `CLion + conan + CMake` , install dependencies before build with CMake

```shell
conan install . -s build_type=Debug --install-folder=cmake-build-debug
conan install . -s build_type=Release --install-folder=cmake-build-release
```

### On Linux

build manually

```shell
mkdir build
cd build
conan install ..
cmake .. -G "Unix Makefiles"  -DCMAKE_BUILD_TYPE=Release 
cmake --build .
```

build with script

```shell
cd scripts
./build.sh
```

### On Windows

build manually

```shell
mkdir build
cd build
conan install ..
cmake .. 
cmake --build . --config Release
```

build with script

```shell
cd scripts
./build.bat
```

### Build with Docker 

```shell
docker build --tag pisano:demo .
docker run -it --name pisano --mount type=bind,source="$(pwd)"/data,target=/retrieve-system/share pisano:demo
```
