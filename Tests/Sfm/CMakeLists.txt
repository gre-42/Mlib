include(../../CMakeCommands.cmake)

my_add_executable(sfm_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(sfm_test MlibSfm)

add_test(NAME SfmTest
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..
    COMMAND $<TARGET_FILE:sfm_test>)
