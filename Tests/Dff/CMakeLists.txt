include(../../CMakeCommands.cmake)

my_add_executable(NAME dff_test RECURSIVE)

target_link_libraries(dff_test PRIVATE MlibGeometry MlibOpenGL)

add_test(NAME DffTest
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND $<TARGET_FILE:dff_test>)
