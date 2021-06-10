cmake_minimum_required(VERSION 3.19)
project(retrieve-system)

set(CMAKE_CXX_STANDARD 14)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_subdirectory(src)

enable_testing()
add_subdirectory(test)

add_executable(testParser test/testParser.cpp)
target_link_libraries(testParser parser)
