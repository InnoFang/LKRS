## build

CLion + conan, install

```shell
conan install . -s build_type=Debug --install-folder=cmake-build-debug
conan install . -s build_type=Release --install-folder=cmake-build-release

```

with MinGW

```
conan install . -s build_type=Debug --install-folder=cmake-build-debug-mingw --build=missing --profile=mingw64
conan install . -s build_type=Release --install-folder=cmake-build-release-mingw --build=missing --profile=mingw64

```


Linux

```shell
mkdir build
cd build
conan install ..
cmake .. -G "Unix Makefiles"  -DCMAKE_BUILD_TYPE=Release && cmake --build .
```

Windows

```shell
mkdir build
cd build
conan install ..
cmake ..
cmake --build . --config Release
```

Docker 

```shell
docker build --tag system:0.1 .
docker run -it --name system --mount type=bind,source="$(pwd)"/data,target=/retrieve-system/share system:0.1
```
