include(../../CMakeCommands.cmake)

my_add_executable(NAME stats_test RECURSIVE)

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(stats_test PRIVATE MlibStats MlibMath MlibMisc)

add_test(NAME StatsTest COMMAND $<TARGET_FILE:stats_test>)
