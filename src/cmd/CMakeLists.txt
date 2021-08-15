add_executable(psoBuild psoBuild.cpp)
target_link_libraries(psoBuild database common ${CONAN_LIBS})

add_executable(psoQuery psoQuery.cpp)
target_link_libraries(psoQuery query parser database common ${CONAN_LIBS})