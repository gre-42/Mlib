include(../../CMakeCommands.cmake)

my_add_executable(plot_pacejkas_magic_formula "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(plot_pacejkas_magic_formula MlibMath)
