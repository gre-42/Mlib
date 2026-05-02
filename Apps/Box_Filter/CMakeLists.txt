include(../../CMakeCommands.cmake)

my_add_executable(NAME box_filter RECURSIVE)

target_link_libraries(box_filter MlibImages)
