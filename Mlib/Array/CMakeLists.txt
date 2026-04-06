include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibArray "1" "" "${excluded_files}")

target_include_directories(MlibArray PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibArray MlibIo)
