include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibMacroExecutor
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_include_directories(MlibMacroExecutor PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibMacroExecutor PUBLIC MlibJson MlibRegex MlibThreads MlibMemory)
