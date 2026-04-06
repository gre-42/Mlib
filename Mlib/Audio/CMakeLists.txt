include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibAudio "1" "" "${excluded_files}")

target_include_directories(MlibAudio PUBLIC ${ALUT_INCLUDE_DIRS})

target_link_libraries(MlibAudio
    MlibMisc
    MlibThreads
    MlibMemory
    MlibTime
    MlibSceneGraph
    MlibMiniMp3
    ${ALUT_LIBRARIES})

if (LAME_FOUND)
    target_link_libraries(MlibAudio ${LAME_LIBRARIES})
endif()
