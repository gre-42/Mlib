include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibOsmLoader "1" "" "${excluded_files}")
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibOsmLoader PRIVATE ../Misc/Precompiled_Header.hpp)
endif()

target_include_directories(
    MlibOsmLoader
    PUBLIC
    ${Mlib_INCLUDE_DIR}
    ${glfw3_INCLUDE_DIR}
    ${RECAST_INCLUDE_DIRS}
    ${DETOUR_INCLUDE_DIRS}
    ${MlibCereal_INCLUDE_DIR})

target_link_libraries(MlibOsmLoader PUBLIC MlibPoly2Tri MlibOpenGL MlibNavigation MlibGeography MlibGeometry MlibMacroExecutor)
target_link_openmp(MlibOsmLoader)
