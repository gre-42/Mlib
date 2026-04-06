include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibStrings "1" "" "${excluded_files}")

target_include_directories(MlibStrings PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibStrings Boost::boost)
