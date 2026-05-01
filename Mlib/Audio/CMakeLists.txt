include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibAudio "1" "" "${excluded_files}")

target_link_libraries(MlibAudio
    PUBLIC
        MlibMisc
        MlibThreads
        MlibMemory
        MlibTime
        MlibSceneGraph
        MlibMiniMp3)

target_compile_definitions(MlibAudio PUBLIC USE_PCM_FILTERS)

if(EMSCRIPTEN)
    target_link_libraries(MlibAudio PRIVATE "-lopenal")
else()
    target_link_libraries(MlibAudio PUBLIC OpenAL::OpenAL)
endif()

if (LAME_FOUND)
    target_link_libraries(MlibAudio PUBLIC ${LAME_LIBRARIES})
endif()
