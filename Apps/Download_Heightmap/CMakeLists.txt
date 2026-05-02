include(../../CMakeCommands.cmake)

my_add_executable(NAME download_heightmap RECURSIVE)

target_link_libraries(download_heightmap PRIVATE MlibMisc MlibImages MlibStb MlibCppHttplib)
