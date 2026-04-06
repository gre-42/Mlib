include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibMath "1" "" "${excluded_files}")

target_include_directories(MlibMath PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibMath MlibOs MlibArray MlibJson)
