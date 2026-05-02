include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibAudio
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_link_libraries(MlibAudio
    PUBLIC
        MlibMap
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
