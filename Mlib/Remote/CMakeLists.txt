include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_library(MlibRemote "1" "" "${mlib_excluded_files}")
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibRemote PRIVATE ../Precompiled_Header.hpp)
    target_include_directories(MlibRemote PRIVATE ${MlibEve_INCLUDE_DIR})
endif()

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(MlibRemote Mlib MlibOs MlibIo MlibMemory)

if (MINGW)
    target_link_libraries(MlibRemote -lws2_32)
endif()
