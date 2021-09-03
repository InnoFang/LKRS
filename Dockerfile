FROM conanio/gcc9:1.39.0

COPY data/lubm /retrieve-system/data/lubm
COPY include /retrieve-system/include
COPY src /retrieve-system/src
COPY test /retrieve-system/test
COPY CMakeLists.txt /retrieve-system/CMakeLists.txt
COPY conanfile.txt /retrieve-system/conanfile.txt

RUN sudo mkdir -p /retrieve-system/bin
RUN sudo mkdir -p /retrieve-system/build
RUN sudo chmod 777 /retrieve-system/*

WORKDIR /retrieve-system/build
RUN conan install ..
RUN sudo cmake .. -DCMAKE_BUILD_TYPE=Release && sudo cmake --build .

WORKDIR /retrieve-system
#RUN sudo ./bin/unitTest
