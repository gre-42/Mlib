include(../../CMakeCommands.cmake)

if (EMSCRIPTEN)
    my_add_library(
        NAME MlibRemote
        RECURSIVE
        ADDITIONAL_FILES ""
        EXCLUDED_FILES "/Config_Server/[^/]+$"
    )
else()
    my_add_library(
        NAME MlibRemote
        RECURSIVE
        ADDITIONAL_FILES ""
        EXCLUDED_FILES "/Web_Transport_Datagram_Node\.cpp$"
    )
endif()

# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibRemote PRIVATE ../Misc/Precompiled_Header.hpp)
    target_include_directories(MlibRemote PRIVATE ${MlibEve_INCLUDE_DIR})
endif()

target_link_libraries(MlibRemote
    PUBLIC
        MlibMisc
        MlibOs
        MlibIo
        MlibMemory
        MlibJson)

if (NOT EMSCRIPTEN)
    target_link_libraries(MlibRemote PUBLIC Boost::boost Boost::url)
else()
    target_link_libraries(MlibRemote PUBLIC AGameHelper)
endif()

if (MINGW)
    target_link_libraries(MlibRemote PUBLIC -lws2_32)
endif()

if (ANDROID)
    target_compile_definitions(MlibRemote PUBLIC BOOST_ASIO_DISABLE_STD_ALIGNED_ALLOC)
endif()
