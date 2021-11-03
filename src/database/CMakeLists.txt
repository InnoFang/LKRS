set(THIS database)

set(SOURCE_FILES
        database.cpp
        legacy/database.cpp)

add_library(${THIS} STATIC ${SOURCE_FILES})
