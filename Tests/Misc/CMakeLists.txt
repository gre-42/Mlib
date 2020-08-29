include(../../CMakeCommands.cmake)

my_add_executable(misc_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(misc_test MlibMath)

add_test(NAME MiscTest COMMAND $<TARGET_FILE:misc_test>)
