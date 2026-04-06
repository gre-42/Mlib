include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibIo "1" "" "${excluded_files}")
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibIo PRIVATE ../Misc/Precompiled_Header.hpp)
endif()

target_include_directories(MlibIo PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibIo MlibMisc MlibOs)
