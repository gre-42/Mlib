include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibImages "1" "" "${excluded_files}")
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibImages PRIVATE ../Misc/Precompiled_Header.hpp)
endif()

target_include_directories(MlibImages PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibImages MlibOs MlibMath MlibStbCpp MlibStats)
