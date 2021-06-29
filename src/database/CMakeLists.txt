set(THIS database)
project(${THIS})

set(SOURCE_FILES
        database.cpp
        DBLoadException.cpp)

add_library(${THIS} STATIC ${SOURCE_FILES})
