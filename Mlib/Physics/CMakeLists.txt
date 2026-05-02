include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibPhysics
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibPhysics PRIVATE ../Misc/Precompiled_Header.hpp)
endif()

target_include_directories(MlibPhysics PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibPhysics
    PUBLIC
        MlibMap
        MlibGeometry
        MlibSceneGraph
        MlibAudio
        MlibThreads
        MlibTime
        MlibMemory)
