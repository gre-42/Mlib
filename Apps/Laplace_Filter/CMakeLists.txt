include(../../CMakeCommands.cmake)

my_add_executable(NAME laplace_filter RECURSIVE)

target_link_libraries(laplace_filter PRIVATE MlibImages MlibGeography)
