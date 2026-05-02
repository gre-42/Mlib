include(../../CMakeCommands.cmake)

my_add_executable(NAME proc_bush RECURSIVE)

target_link_libraries(proc_bush PRIVATE MlibMisc)
