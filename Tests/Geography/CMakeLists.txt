include(../../CMakeCommands.cmake)

my_add_executable(NAME geography_test RECURSIVE)

target_include_directories(geography_test PRIVATE Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(geography_test PRIVATE MlibGeography MlibImages)

add_test(NAME MlibGeography COMMAND $<TARGET_FILE:geography_test>)
