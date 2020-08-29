include(../../CMakeCommands.cmake)

my_add_executable(stats_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(stats_test MlibMath)

add_test(NAME StatsTest COMMAND $<TARGET_FILE:stats_test>)
