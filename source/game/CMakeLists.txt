find_package(SFML REQUIRED system window graphics)
include_directories(${SFML_INCLUDE_DIR})

add_executable(game
    main.cpp
)

target_link_libraries(game ${SFML_LIBRARIES})
