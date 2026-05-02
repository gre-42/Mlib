include(../../CMakeCommands.cmake)

my_add_executable(NAME proc_tree RECURSIVE)

target_link_libraries(proc_tree PRIVATE MlibOs MlibProctree)
