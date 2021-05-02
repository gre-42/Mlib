include(../../CMakeCommands.cmake)

my_add_executable(floating_point_exceptions_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(floating_point_exceptions_test Mlib)

add_test(NAME FloatingPointExceptionsTest COMMAND $<TARGET_FILE:floating_point_exceptions_test>)
