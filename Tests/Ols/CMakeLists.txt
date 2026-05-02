include(../../CMakeCommands.cmake)

my_add_executable(NAME ols_test RECURSIVE)

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(ols_test MlibMath)

add_test(NAME OlsTest COMMAND $<TARGET_FILE:ols_test>)
