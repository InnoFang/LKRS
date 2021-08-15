set(THIS query)

set(SOURCE_FILES
        sparql_query.cpp
        query_plan.cpp
        )

add_library(${THIS} STATIC ${SOURCE_FILES})
