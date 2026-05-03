include(../../CMakeCommands.cmake)

if (ANDROID)
    my_add_library(
        NAME AGameHelper
        RECURSIVE
        ADDITIONAL_FILES ""
        EXCLUDED_FILES "/Emscripten/[^/]+$"
    )
elseif (EMSCRIPTEN)
    my_add_library(
        NAME AGameHelper
        RECURSIVE
        ADDITIONAL_FILES ""
        EXCLUDED_FILES "/Android/[^/]+$"
    )
else()
    message(FATAL_ERROR "Unknown platform type")
endif()

target_link_libraries(AGameHelper PUBLIC MlibOs)
