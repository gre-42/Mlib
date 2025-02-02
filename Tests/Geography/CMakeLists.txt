include(../../CMakeCommands.cmake)

my_add_executable(geography_test "1")

target_include_directories(geography_test PRIVATE Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(geography_test MlibGeography MlibImages)

add_test(NAME MlibGeography COMMAND $<TARGET_FILE:geography_test>)
