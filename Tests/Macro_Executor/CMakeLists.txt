include(../../CMakeCommands.cmake)

my_add_executable(macro_executor_test "1")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(macro_executor_test MlibMacroExecutor)

add_test(
    NAME MacroExecutorTest
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND $<TARGET_FILE:macro_executor_test>)
