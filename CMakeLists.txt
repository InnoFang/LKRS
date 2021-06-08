cmake_minimum_required(VERSION 3.19)
project(retrieve-system)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(retrieve-system test/main.cpp)