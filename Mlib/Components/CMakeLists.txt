include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibComponents "1" "" "${mlib_excluded_files}")
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibComponents PRIVATE ../Precompiled_Header.hpp)
endif()

target_link_libraries(MlibComponents MlibPhysics MlibSceneGraph)
