include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibRender "1" "" "${mlib_excluded_files}")
target_precompile_headers(MlibRender PRIVATE ../Precompiled_Header.hpp)

target_include_directories(MlibRender PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibRender Mlib MlibImages MlibSceneGraph MlibStb MlibTime MlibLayout MlibGlad)
