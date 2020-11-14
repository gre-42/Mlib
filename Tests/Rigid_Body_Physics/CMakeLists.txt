include(../../CMakeCommands.cmake)

my_add_executable(rigid_body_physics_test "1")

include_directories(${Mlib_INCLUDE_DIR})

# target_link_libraries(rigid_body_physics_test MlibPhysics)

add_test(NAME RigidBodyPhysicsTest COMMAND $<TARGET_FILE:rigid_body_physics_test>)
