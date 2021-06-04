include(../../CMakeCommands.cmake)

my_add_executable(chessboard_detector_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(chessboard_detector_test MlibSfm)

add_test(NAME ChessboardDetectorTest
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND $<TARGET_FILE:chessboard_detector_test>)
