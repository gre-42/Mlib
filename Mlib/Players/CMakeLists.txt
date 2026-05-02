include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibPlayers
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibPlayers PRIVATE ../Misc/Precompiled_Header.hpp)
endif()

target_link_libraries(MlibPlayers
    PUBLIC
        MlibMap
        MlibPhysics
        MlibComponents)
