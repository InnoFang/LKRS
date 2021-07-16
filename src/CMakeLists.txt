cmake_minimum_required(VERSION 3.19)
project(source)

add_subdirectory(common)
add_subdirectory(database)
add_subdirectory(parser)
add_subdirectory(query)

#set(SRC_DIR .)
#foreach(SUB_SRC_DIRS ${SRC_DIR})
#    message("${SUB_SRC_DIRS}")
#    add_subdirectory("${SUB_SRC_DIRS}")
#endforeach()

