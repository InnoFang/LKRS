FROM conanio/gcc9

# setup conan
USER root
RUN conan profile new default --detect
RUN conan profile update settings.compiler.libcxx=libstdc++11 default

#RUN pip3 install conan
#RUN conan user &&\
#    conan profile new --detect default &&\
#    conan profile update settings.compiler.libcxx=libstdc++11 default

COPY data/lubm /retrieve-system/data/lubm
COPY include /retrieve-system/include
COPY src /retrieve-system/src
COPY test /retrieve-system/test
COPY CMakeLists.txt /retrieve-system/CMakeLists.txt
COPY conanfile.txt /retrieve-system/conanfile.txt

RUN mkdir -p /retrieve-system/bin
RUN mkdir -p /retrieve-system/build

WORKDIR /retrieve-system/build
RUN conan install .. -s build_type=Release --build
RUN cmake .. -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
RUN cmake --build .

WORKDIR /retrieve-system
RUN ./bin/unitTests
