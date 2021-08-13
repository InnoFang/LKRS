set(THIS parser)
project(${THIS})

set(SOURCE_FILES
        sparql_parser.cpp
        )

add_library(${THIS} STATIC ${SOURCE_FILES})
