include(../../CMakeCommands.cmake)

my_add_executable(box_filter_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(box_filter_test MlibImages)

add_test(NAME BoxFilterTest COMMAND $<TARGET_FILE:box_filter_test>)
