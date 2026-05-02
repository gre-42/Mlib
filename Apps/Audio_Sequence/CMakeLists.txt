include(../../CMakeCommands.cmake)

my_add_executable(NAME audio_sequence RECURSIVE)

target_link_libraries(audio_sequence PRIVATE MlibAudio MlibStrings)
