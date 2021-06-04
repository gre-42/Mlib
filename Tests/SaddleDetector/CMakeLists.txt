include(../../CMakeCommands.cmake)

my_add_executable(saddle_detector_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(saddle_detector_test MlibImages)

add_test(NAME SaddleDetectorTest
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND $<TARGET_FILE:saddle_detector_test>)
