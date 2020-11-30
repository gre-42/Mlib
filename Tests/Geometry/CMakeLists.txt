include(../../CMakeCommands.cmake)

my_add_executable(geometry_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(geometry_test MlibGeometry)

add_test(NAME GeometryTest COMMAND $<TARGET_FILE:geometry_test>)
