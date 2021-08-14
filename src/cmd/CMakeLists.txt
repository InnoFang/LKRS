add_executable(psoBuild psoBuild.cpp)
target_link_libraries(psoBuild database common ${CONAN_LIBS})
