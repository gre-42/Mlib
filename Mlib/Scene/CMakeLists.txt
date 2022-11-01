include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_shared_library(MlibScene "1" "" "${mlib_excluded_files}")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set (fs_lib stdc++fs)
else()
    set (fs_lib "")
endif()
target_link_libraries(MlibScene MlibPhysics MlibPlayers MlibRender MlibAudio MlibOsmLoader MlibSceneGraph MlibMacroExecutor ${fs_lib})
