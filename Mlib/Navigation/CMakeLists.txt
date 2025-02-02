include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibNavigation "1" "" "${mlib_excluded_files}")

include_directories(${Mlib_INCLUDE_DIR} ${RECAST_INCLUDE_DIRS} ${DETOUR_INCLUDE_DIRS})

target_link_libraries(MlibNavigation MlibGeometry ${RECAST_LIBRARIES} ${DETOUR_LIBRARIES} ${DEBUGUTILS_LIBRARIES})
