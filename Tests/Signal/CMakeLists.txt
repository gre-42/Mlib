include(../../CMakeCommands.cmake)

my_add_executable(signal_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(signal_test MlibOs Mlib)

add_test(NAME StatsTest COMMAND $<TARGET_FILE:signal_test>)
