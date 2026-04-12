if (ANDROID)
    include(AndroidNdkModules)
    android_ndk_import_module_native_app_glue()
    # https://stackoverflow.com/questions/11801186/cmake-unable-to-determine-linker-language-with-c
    set_target_properties(native_app_glue PROPERTIES LINKER_LANGUAGE CXX)
endif()

include(../../CMakeCommands.cmake)

set(excluded_files "")

if (ANDROID)
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
    set_target_properties(MlibOs
        PROPERTIES
            CXX_STANDARD 20
            CXX_STANDARD_REQUIRED YES
            CXX_EXTENSIONS NO
            INTERFACE_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:MlibOs,INCLUDE_DIRECTORIES>
        )
else()
    my_add_library(MlibOs "0" "" "${excluded_files}")
endif()

target_link_libraries(MlibOs PUBLIC MlibStrings Boost::regex)
