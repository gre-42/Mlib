include(../../CMakeCommands.cmake)

my_add_executable(NAME repackage_kn5 RECURSIVE)

target_link_libraries(repackage_kn5 PRIVATE MlibOpenGL)
