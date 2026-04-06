include(../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibStbCpp "1" "" "${excluded_files}")
target_include_directories(MlibStbCpp PUBLIC ${Mlib_INCLUDE_DIR})
target_link_libraries(MlibStbCpp MlibStb MlibMisc MlibArray)
