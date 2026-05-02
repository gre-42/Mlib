include(../../CMakeCommands.cmake)

my_add_executable(NAME create_navigation_mesh RECURSIVE)

include_directories(${Mlib_INCLUDE_DIR} ${RECAST_INCLUDE_DIRS} ${DETOUR_INCLUDE_DIRS})

target_link_libraries(create_navigation_mesh PRIVATE MlibNavigation)
