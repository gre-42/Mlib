include(../../CMakeCommands.cmake)

my_add_executable(NAME floating_point_exceptions_test RECURSIVE)

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(floating_point_exceptions_test PRIVATE MlibMisc)

add_test(NAME FloatingPointExceptionsTest COMMAND $<TARGET_FILE:floating_point_exceptions_test>)
