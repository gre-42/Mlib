include(../../CMakeCommands.cmake)

my_add_executable(audio_cross_fade "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(audio_cross_fade MlibAudio MlibStrings)
