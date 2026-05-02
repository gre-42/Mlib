include(../../CMakeCommands.cmake)

if (ANDROID)
    my_add_shared_library(
        NAME RenderSceneFileActivity
        RECURSIVE
        ADDITIONAL_FILES ""
        EXCLUDED_FILES "")
elseif (EMSCRIPTEN)
    my_add_executable(NAME RenderSceneFileActivity RECURSIVE)
else()
    message(FATAL_ERROR "Unknown target")
endif()

target_link_libraries(RenderSceneFileActivity PRIVATE 
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,MlibScene>"
    AGameHelper
)

if (ANDROID)
    # Export ANativeActivity_onCreate(),
    # Refer to: https://github.com/android-ndk/ndk/issues/381.
    target_link_options(RenderSceneFileActivity PRIVATE "-u" "ANativeActivity_onCreate")
endif()

if (EMSCRIPTEN)
    target_link_options(RenderSceneFileActivity PRIVATE -sFORCE_FILESYSTEM=1)
endif()
