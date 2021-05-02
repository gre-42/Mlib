include(../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_shared_library(MlibStbImage "1" "" "${mlib_excluded_files}")
target_link_libraries(MlibStbImage Mlib)
