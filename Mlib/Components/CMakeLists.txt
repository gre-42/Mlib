include(../../CMakeCommands.cmake)

my_add_library(
    NAME MlibComponents
    RECURSIVE
    ADDITIONAL_FILES ""
    EXCLUDED_FILES ""
)
# https://issuetracker.google.com/issues/214811726
if (NOT ANDROID)
    target_precompile_headers(MlibComponents PRIVATE ../Misc/Precompiled_Header.hpp)
endif()

target_link_libraries(MlibComponents MlibPhysics MlibSceneGraph)
