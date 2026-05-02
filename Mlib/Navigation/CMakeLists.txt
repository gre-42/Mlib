include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibNavigation
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_include_directories(MlibNavigation
    PUBLIC
        ${Mlib_INCLUDE_DIR}
        ${RECAST_INCLUDE_DIRS}
        ${DETOUR_INCLUDE_DIRS})

target_link_libraries(MlibNavigation
    PUBLIC
        MlibGeometry
        ${RECAST_LIBRARIES}
        ${DETOUR_LIBRARIES}
        ${DEBUGUTILS_LIBRARIES})
