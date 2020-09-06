include(../../CMakeCommands.cmake)

my_add_executable(scene_test "1")

include_directories(${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

target_link_libraries(scene_test MlibScene)

add_test(NAME SceneTest
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..
    COMMAND $<TARGET_FILE:scene_test>)
