include(../../CMakeCommands.cmake)

my_add_executable(physics_test "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(physics_test MlibPhysics)

add_test(NAME PhysicsTest COMMAND $<TARGET_FILE:physics_test>)
