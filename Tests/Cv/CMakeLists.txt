include(../../CMakeCommands.cmake)

my_add_executable(cv_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(cv_test MlibCv)

add_test(NAME CvTest
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND $<TARGET_FILE:cv_test>)
