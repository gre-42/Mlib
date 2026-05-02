include(../../CMakeCommands.cmake)

my_add_executable(NAME macro_executor_test RECURSIVE)

target_link_libraries(macro_executor_test MlibMacroExecutor)

add_test(
    NAME MacroExecutorTest
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND $<TARGET_FILE:macro_executor_test>)
