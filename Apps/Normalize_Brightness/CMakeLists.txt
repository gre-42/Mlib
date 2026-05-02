include(../../CMakeCommands.cmake)

my_add_executable(NAME normalize_brightness RECURSIVE)

target_link_libraries(normalize_brightness MlibImages)
