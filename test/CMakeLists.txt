file(GLOB SOURCE_FILES  *.cpp)
source_group("Source" FILES ${SOURCE_FILES})

add_executable(unitTests ${SOURCE_FILES})
target_link_libraries(unitTests parser common ${CONAN_LIBS})
add_test(
        NAME unitTests
        COMMAND unitTests
)