add_executable(testParser testParser.cpp)
target_link_libraries(testParser parser common ${CONAN_LIBS})
