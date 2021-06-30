set(THIS query)
project(${THIS})

set(SOURCE_FILES
        SPARQLQuery.cpp
        )

add_library(${THIS} STATIC ${SOURCE_FILES})
