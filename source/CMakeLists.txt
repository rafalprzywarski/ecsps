add_definitions("-Wall -Wno-unused-local-typedef")
include_directories(".")

add_subdirectory("core")
add_subdirectory("benchmarks")
add_subdirectory("game")
add_subdirectory("test")
