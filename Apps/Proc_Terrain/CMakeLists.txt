include(../../CMakeCommands.cmake)

my_add_executable(NAME proc_terrain RECURSIVE)

target_link_libraries(proc_terrain PRIVATE MlibImages)
