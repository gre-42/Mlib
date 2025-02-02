include(../../CMakeCommands.cmake)

my_add_executable(proc_terrain "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(proc_terrain MlibImages)
