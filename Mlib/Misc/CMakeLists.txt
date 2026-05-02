include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibMisc "1" "" "${excluded_files}")

target_include_directories(MlibMisc PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibMisc MlibStrings MlibOs MlibThreads)
