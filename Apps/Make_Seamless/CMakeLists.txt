include(../../CMakeCommands.cmake)

my_add_executable(make_seamless "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(make_seamless MlibImages)
