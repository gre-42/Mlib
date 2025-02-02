include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibImages "1" "" "${mlib_excluded_files}")
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibImages PRIVATE ../Precompiled_Header.hpp)
endif()

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(MlibImages MlibOs MlibMath MlibStbCpp)
