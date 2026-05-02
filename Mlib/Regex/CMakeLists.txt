include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibRegex
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)

target_include_directories(MlibRegex PUBLIC ${Mlib_INCLUDE_DIR})

target_link_libraries(MlibRegex MlibOs)
if (MSVC)
    target_link_libraries(MlibRegex Boost::boost)
endif()
