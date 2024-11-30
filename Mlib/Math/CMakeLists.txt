include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibMath "1" "" "${mlib_excluded_files}")

target_include_directories(
    MlibMath PUBLIC ${Mlib_INCLUDE_DIR} ${MlibEve_INCLUDE_DIR})

target_link_libraries(MlibMath MlibOs MlibArray MlibJson)
