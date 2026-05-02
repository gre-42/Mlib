include(../../CMakeCommands.cmake)

my_add_executable(NAME proc_bush_svg RECURSIVE)

target_link_libraries(proc_bush_svg MlibMisc)
