cmake_minimum_required(VERSION 3.19)
project(source)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(common)
add_subdirectory(database)
add_subdirectory(parser)
add_subdirectory(main)

#target_link_libraries(source parser)
#set(SRC parser)
#set(SRC_DIR src)
#foreach(SUB_SRC_DIRS ${SRC})
#    add_subdirectory("${SRC_DIR}/${SUB_SRC_DIRS}")
#endforeach(LIBRARY)

#aux_source_directory(. SRC_LIST)
#add_library(parser SHARED ${SRC_LIST})
