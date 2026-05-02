include(../../CMakeCommands.cmake)

my_add_executable(NAME array_test RECURSIVE)

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(array_test PRIVATE MlibMisc MlibMath MlibStrings)

add_test(NAME ArrayTest COMMAND $<TARGET_FILE:array_test>)
