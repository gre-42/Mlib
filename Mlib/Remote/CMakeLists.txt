include(../../CMakeCommands.cmake)

set(excluded_files "")

my_add_library(MlibRemote "1" "" "${excluded_files}")
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibRemote PRIVATE ../Misc/Precompiled_Header.hpp)
    target_include_directories(MlibRemote PRIVATE ${MlibEve_INCLUDE_DIR})
endif()

include_directories(${Mlib_INCLUDE_DIR})

target_link_libraries(MlibRemote MlibMisc MlibOs MlibIo MlibMemory MlibJson)

if (NOT EMSCRIPTEN)
    target_link_libraries(MlibRemote Boost::boost)
endif()

if (MINGW)
    target_link_libraries(MlibRemote -lws2_32)
endif()

if (ANDROID)
    target_compile_definitions(MlibRemote PUBLIC BOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC)
endif()
