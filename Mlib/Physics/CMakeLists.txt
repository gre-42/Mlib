include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibPhysics "1" "" "${excluded_files}")
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibPhysics PRIVATE ../Misc/Precompiled_Header.hpp)
endif()

target_include_directories(MlibPhysics PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibPhysics MlibGeometry MlibSceneGraph MlibAudio MlibThreads MlibTime MlibMemory)
