include(../../CMakeCommands.cmake)

my_add_executable(NAME make_seamless RECURSIVE)

target_link_libraries(make_seamless PRIVATE MlibImages)
