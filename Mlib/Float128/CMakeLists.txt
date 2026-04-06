include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibFloat128 "1" "" "${excluded_files}")

target_include_directories(MlibFloat128 PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibFloat128 -lquadmath)
