cmake_minimum_required(VERSION 3.19)
project(retrieve-system)

set(CMAKE_CXX_STANDARD 17)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_CXX_FLAGS "${CAMKE_CXX_FLAGS} -pthread")

include_directories(include)

add_subdirectory(src)

add_executable(psoBuild src/cmd/psoBuild.cpp)
target_link_libraries(psoBuild database common)

add_executable(psoQuery src/cmd/psoQuery.cpp)
target_link_libraries(psoQuery query parser database common)

enable_testing()
add_subdirectory(test)

add_executable(testParser test/testParser.cpp)
target_link_libraries(testParser parser common)
