include(../../CMakeCommands.cmake)

my_add_executable(audio_sequence "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(audio_sequence MlibAudio MlibStrings)
