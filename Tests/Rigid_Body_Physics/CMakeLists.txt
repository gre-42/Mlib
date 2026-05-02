include(../../CMakeCommands.cmake)

my_add_executable(NAME rigid_body_physics_test RECURSIVE)

target_link_libraries(rigid_body_physics_test MlibPhysics)

add_test(NAME RigidBodyPhysicsTest COMMAND $<TARGET_FILE:rigid_body_physics_test>)
