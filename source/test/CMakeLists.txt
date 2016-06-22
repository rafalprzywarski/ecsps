include_directories(${CML_INCLUDE_DIRS})
include_directories(${GoogleMock_INCLUDE_DIRS})
include_directories("../core")

add_executable(ecsps_test
    ecsps/KeywordTest.cpp
    ecsps/ResourcePoolTest.cpp
    ecsps/ValuePoolTest.cpp
    main.cpp
)

target_link_libraries(ecsps_test ecsps_core ${Boost_LIBRARIES} gmock pthread)
