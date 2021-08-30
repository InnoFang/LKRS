## build

CLion + conan, install

```shell
conan install . -s build_type=Debug --install-folder=cmake-build-debug
conan install . -s build_type=Release --install-folder=cmake-build-release

```


Linux

```shell
mkdir build
cd build
conan install ..
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

Windows

```shell
mkdir build
cd build
conan install ..
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```
