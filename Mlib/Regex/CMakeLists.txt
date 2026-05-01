include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibRegex "1" "" "${excluded_files}")

target_include_directories(MlibRegex PUBLIC ${Mlib_INCLUDE_DIR})

if (MSVC)
    target_link_libraries(MlibRegex Boost::boost)
endif()
