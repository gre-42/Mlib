include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibScene "1" "" "${mlib_excluded_files}")

target_link_libraries(MlibScene MlibPhysics MlibPlayers MlibRender MlibOsmLoader MlibSceneGraph MlibMacroExecutor)
if (OPENAL_FOUND AND ALUT_FOUND)
    target_link_libraries(MlibScene MlibAudio)
endif()
