include(../../CMakeCommands.cmake)

my_add_executable(NAME plot_pacejkas_magic_formula RECURSIVE)

target_link_libraries(plot_pacejkas_magic_formula MlibMath)
