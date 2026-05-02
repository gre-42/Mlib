include(../../CMakeCommands.cmake)

my_add_executable(NAME math_test RECURSIVE)

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(math_test MlibMisc MlibMath MlibTime)

add_test(NAME MathTest COMMAND $<TARGET_FILE:math_test>)
