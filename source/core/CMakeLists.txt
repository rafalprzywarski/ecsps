include_directories(${CML_INCLUDE_DIRS})
include_directories("../core")

add_library(ecsps_core
    ecsps/Keyword.cpp
    ecsps/dummy.cpp
)
