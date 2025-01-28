include(../../CMakeCommands.cmake)

my_add_executable(geometry_test "1")

target_include_directories(geometry_test PRIVATE Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(geometry_test MlibGeometry MlibPoly2Tri)

add_test(NAME GeometryTest COMMAND $<TARGET_FILE:geometry_test>)
