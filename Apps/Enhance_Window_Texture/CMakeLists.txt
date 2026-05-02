include(../../CMakeCommands.cmake)

my_add_executable(NAME enhance_window_texture RECURSIVE)

target_link_libraries(enhance_window_texture MlibMisc MlibStbCpp)
