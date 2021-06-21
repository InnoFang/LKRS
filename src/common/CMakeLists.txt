set(THIS common)
project(${THIS})

set(SOURCE_FILES
        triple.cpp
        )

add_library(${THIS} STATIC ${SOURCE_FILES})
