include(../../CMakeCommands.cmake)

my_add_executable(NAME signal_test RECURSIVE)

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(signal_test PRIVATE MlibOs MlibMisc)

add_test(NAME StatsTest COMMAND $<TARGET_FILE:signal_test>)
