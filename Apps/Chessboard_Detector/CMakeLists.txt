include(../../CMakeCommands.cmake)

my_add_executable(chessboard_detector "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(chessboard_detector MlibSfm)
