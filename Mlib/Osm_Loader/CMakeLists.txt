include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibOsmLoader "1" "" "${mlib_excluded_files}")

include_directories(${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR} ${RECAST_INCLUDE_DIRS} ${DETOUR_INCLUDE_DIRS})

target_link_libraries(MlibOsmLoader MlibPoly2Tri MlibRender MlibNavigation MlibGeography MlibJson)
