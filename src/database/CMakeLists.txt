set(THIS database)
project(${THIS})

set(SOURCE_FILES
        database.cpp
        )

add_library(${THIS} STATIC ${SOURCE_FILES})
