include(../../CMakeCommands.cmake)

my_add_executable(math_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(math_test MlibMath)

add_test(NAME MathTest COMMAND $<TARGET_FILE:math_test>)
