include(../../CMakeCommands.cmake)

my_add_executable(NAME proc_terrain_perlin RECURSIVE)

target_link_libraries(proc_terrain_perlin MlibImages)
