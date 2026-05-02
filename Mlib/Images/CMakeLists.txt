include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibImages
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibImages PRIVATE ../Misc/Precompiled_Header.hpp)
endif()

target_include_directories(MlibImages PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibImages MlibOs MlibMath MlibStbCpp MlibStats MlibMisc)
