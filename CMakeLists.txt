cmake_minimum_required(VERSION 3.10)

project(retrieve-system LANGUAGES CXX) # CUDA
# if need to set CUDA compiler, add
# -DCMAKE_CUDA_COMPILER="C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v10.1/bin/nvcc.exe"


if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
else()
    message(WARNING "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif()

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 14)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

include_directories(include)
add_subdirectory(src)

add_executable(psoBuild src/cmd/psoBuild.cpp)
target_link_libraries(psoBuild database common ${CONAN_LIBS})

add_executable(psoQuery src/cmd/psoQuery.cpp)
target_link_libraries(psoQuery query parser database common ${CONAN_LIBS})

#enable_testing()
#add_subdirectory(test)
#
#add_executable(testParser test/testParser.cpp)
#target_link_libraries(testParser parser common)
