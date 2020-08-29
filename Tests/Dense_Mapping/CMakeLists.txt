include(../../CMakeCommands.cmake)

my_add_executable(dense_mapping_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(dense_mapping_test MlibSfm)

add_test(NAME DenseMappingTest
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..
    COMMAND $<TARGET_FILE:dense_mapping_test>)
