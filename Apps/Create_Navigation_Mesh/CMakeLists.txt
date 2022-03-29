include(../../CMakeCommands.cmake)

my_add_executable(create_navigation_mesh "1")

include_directories(${Mlib_INCLUDE_DIR} ${RECAST_INCLUDE_DIRS} ${DETOUR_INCLUDE_DIRS})

target_link_libraries(create_navigation_mesh MlibNavigation)
