find_package(SFML REQUIRED system window graphics)
include_directories(${SFML_INCLUDE_DIR})
include_directories(${CML_INCLUDE_DIRS})
include_directories("../core")

add_executable(game
    main.cpp
)

target_link_libraries(game ecsps_core ${SFML_LIBRARIES})
