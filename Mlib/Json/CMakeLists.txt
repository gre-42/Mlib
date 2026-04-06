include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibJson "1" "" "${excluded_files}")

target_include_directories(MlibJson PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibJson PUBLIC MlibMisc MlibOs)
