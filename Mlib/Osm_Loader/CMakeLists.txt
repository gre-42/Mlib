include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibOsmLoader "1" "" "${mlib_excluded_files}")
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibOsmLoader PRIVATE ../Precompiled_Header.hpp)
endif()

target_include_directories(
    MlibOsmLoader
    PUBLIC
    ${Mlib_INCLUDE_DIR}
    ${glfw3_INCLUDE_DIR}
    ${RECAST_INCLUDE_DIRS}
    ${DETOUR_INCLUDE_DIRS}
    ${MlibCereal_INCLUDE_DIR})

target_link_libraries(MlibOsmLoader MlibPoly2Tri MlibRender MlibNavigation MlibGeography MlibGeometry MlibMacroExecutor)
