include(../../CMakeCommands.cmake)

my_add_executable(NAME audio_cross_fade RECURSIVE)

target_link_libraries(audio_cross_fade MlibAudio MlibStrings)
