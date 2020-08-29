include(../../CMakeCommands.cmake)

my_add_executable(render_test "1")

include_directories(${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

target_link_libraries(render_test MlibRender)

add_test(NAME RenderTest
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..
    COMMAND $<TARGET_FILE:render_test>)
