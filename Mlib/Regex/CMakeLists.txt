include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibRegex "1" "" "${mlib_excluded_files}")

include_directories(${Mlib_INCLUDE_DIR})

if (ANDROID)
    target_link_libraries(MlibRegex MlibOs)
endif()
