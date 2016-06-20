include_directories(${CML_INCLUDE_DIRS})
include_directories(${GoogleMock_INCLUDE_DIRS})
include_directories("../core")

add_executable(ecsps_test
    main.cpp
)

target_link_libraries(ecsps_test ecsps_core ${Boost_LIBRARIES} gmock pthread)
