set(THIS parser)
project(${THIS})

set(SOURCE_FILES
        SPARQLParser.cpp
        )

add_library(${THIS} STATIC ${SOURCE_FILES})
