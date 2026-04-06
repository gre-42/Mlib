include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibMacroExecutor "1" "" "${excluded_files}")

target_include_directories(MlibMacroExecutor PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibMacroExecutor MlibJson MlibRegex MlibThreads MlibMemory)
