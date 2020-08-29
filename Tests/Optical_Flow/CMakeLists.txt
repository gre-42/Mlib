include(../../CMakeCommands.cmake)

my_add_executable(optical_flow_test "1")

include_directories(Mlib ${Mlib_INCLUDE_DIR})

target_link_libraries(optical_flow_test MlibImages)

add_test(NAME OpticalFlowTest COMMAND $<TARGET_FILE:optical_flow_test>)
