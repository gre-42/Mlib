if (ANDROID)
    include(AndroidNdkModules)
    android_ndk_import_module_native_app_glue()
    # https://stackoverflow.com/questions/11801186/cmake-unable-to-determine-linker-language-with-c
    set_target_properties(native_app_glue PROPERTIES LINKER_LANGUAGE CXX)
endif()

include(../../CMakeCommands.cmake)

if (ANDROID)
    my_add_library(
        NAME MlibOs
        RECURSIVE
        ADDITIONAL_FILES ""
        EXCLUDED_FILES "/Emscripten/[^/]+$"
    )
    target_link_libraries(MlibOs
        PUBLIC
            native_app_glue
            GLESv3
            EGL
            log
            android
            atomic
        )
elseif (EMSCRIPTEN)
    my_add_library(
        NAME MlibOs
        RECURSIVE
        ADDITIONAL_FILES ""
        EXCLUDED_FILES "/Android/[^/]+$"
)
else()
    my_add_library(
        NAME MlibOs
        RECURSIVE
        ADDITIONAL_FILES ""
        EXCLUDED_FILES "/(Android|Emscripten)/[^/]+$"
)
endif()

target_link_libraries(MlibOs
    PUBLIC
        MlibStrings)
