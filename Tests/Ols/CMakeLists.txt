include(../../CMakeCommands.cmake)

my_add_executable(ols_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(ols_test MlibMath)

add_test(NAME OlsTest COMMAND $<TARGET_FILE:ols_test>)
