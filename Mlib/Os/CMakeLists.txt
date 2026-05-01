if (ANDROID)
    include(AndroidNdkModules)
    android_ndk_import_module_native_app_glue()
    # https://stackoverflow.com/questions/11801186/cmake-unable-to-determine-linker-language-with-c
    set_target_properties(native_app_glue PROPERTIES LINKER_LANGUAGE CXX)
endif()

include(../../CMakeCommands.cmake)


if (ANDROID)
    set(excluded_files "/Emscripten/[^/]+$")
    my_add_library(MlibOs "1" "" "${excluded_files}")
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
    set(excluded_files "/Android/[^/]+$")
    my_add_library(MlibOs "1" "" "${excluded_files}")
else()
    set(excluded_files "/(Android|Emscripten)/[^/]+$")
    my_add_library(MlibOs "1" "" "${excluded_files}")
endif()

target_link_libraries(MlibOs PUBLIC MlibStrings)
