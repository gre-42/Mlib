include(../../CMakeCommands.cmake)

my_add_executable(compress_images "1")

target_link_libraries(compress_images MlibImages MlibAudio MlibJson)
