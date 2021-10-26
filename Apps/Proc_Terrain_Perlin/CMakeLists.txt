include(../../CMakeCommands.cmake)

my_add_executable(proc_terrain_perlin "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(proc_terrain_perlin MlibImages)
