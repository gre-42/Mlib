include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibIo
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibIo PRIVATE ../Misc/Precompiled_Header.hpp)
endif()

target_include_directories(MlibIo PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibIo MlibMisc MlibOs)
