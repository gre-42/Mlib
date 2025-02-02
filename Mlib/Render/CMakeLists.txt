include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibRender "1" "" "${mlib_excluded_files}")
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibRender PRIVATE ../Precompiled_Header.hpp)
endif()

target_include_directories(MlibRender PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibRender MlibStats MlibImages MlibSceneGraph MlibStb MlibTime MlibLayout MlibGlad MlibMemory Mlib)
