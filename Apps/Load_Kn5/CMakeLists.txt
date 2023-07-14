include(../../CMakeCommands.cmake)

my_add_executable(load_kn5 "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(load_kn5 MlibGeometry)
