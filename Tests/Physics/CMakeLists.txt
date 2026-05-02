include(../../CMakeCommands.cmake)

my_add_executable(NAME physics_test RECURSIVE)

target_link_libraries(physics_test PRIVATE MlibPhysics)

add_test(NAME PhysicsTest COMMAND $<TARGET_FILE:physics_test>)
