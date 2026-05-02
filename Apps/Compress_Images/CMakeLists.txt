include(../../CMakeCommands.cmake)

my_add_executable(NAME compress_images RECURSIVE)

target_link_libraries(compress_images MlibImages MlibAudio MlibJson)
