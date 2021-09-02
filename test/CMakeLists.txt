add_executable(testParser sparql_parser_unittest.cpp)
target_link_libraries(testParser parser common ${CONAN_LIBS})
