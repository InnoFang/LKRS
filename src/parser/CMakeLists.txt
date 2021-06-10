project(parser)

set(SOURCE_FILES
        SPARQLParser.cpp
        triple.cpp
)

add_library(parser SHARED STATIC ${SOURCE_FILES})
