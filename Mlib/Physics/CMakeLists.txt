include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibPhysics "1" "" "${mlib_excluded_files}")
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibPhysics PRIVATE ../Precompiled_Header.hpp)
endif()

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(MlibPhysics MlibGeometry MlibSceneGraph MlibAudio MlibThreads MlibTime)
