include(../../CMakeCommands.cmake)

my_add_executable(dff_test "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(dff_test MlibGeometry MlibRender)

add_test(NAME DffTest
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND $<TARGET_FILE:dff_test>)
