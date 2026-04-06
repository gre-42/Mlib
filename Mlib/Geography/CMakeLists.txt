include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibGeography "1" "" "${excluded_files}")

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(MlibGeography MlibStbCpp MlibImages)
