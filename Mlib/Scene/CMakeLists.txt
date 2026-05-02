include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibScene
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibScene PRIVATE ../Misc/Precompiled_Header.hpp)
endif()

target_link_libraries(MlibScene
    PUBLIC
        MlibPhysics
        MlibPlayers
        MlibOpenGL
        MlibOsmLoader
        MlibSceneGraph
        MlibMacroExecutor
        MlibRemote)
if (OPENAL_FOUND)
    target_link_libraries(MlibScene
        PUBLIC
            MlibAudio)
endif()
