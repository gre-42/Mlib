include(../../CMakeCommands.cmake)

my_add_executable(physics_test "1")

include_directories(${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

target_link_libraries(physics_test MlibScene)

add_test(NAME PhysicsTest
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..
    COMMAND $<TARGET_FILE:physics_test>)
