include(../../CMakeCommands.cmake)

my_add_executable(NAME audio_loop RECURSIVE)

target_link_libraries(audio_loop PRIVATE MlibAudio MlibStrings)
