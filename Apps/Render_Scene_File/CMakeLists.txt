include(../../CMakeCommands.cmake)

if (ANDROID)
    set(mlib_excluded_files "")

    my_add_shared_library(RenderSceneFileActivity "1" "" "${mlib_excluded_files}")

    target_include_directories(RenderSceneFileActivity PRIVATE ${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

    target_link_libraries(RenderSceneFileActivity PRIVATE MlibScene AGameHelper)

    # Export ANativeActivity_onCreate(),
    # Refer to: https://github.com/android-ndk/ndk/issues/381.
    set_target_properties(RenderSceneFileActivity
            PROPERTIES  LINK_FLAGS "-u ANativeActivity_onCreate")
else()
    my_add_executable(render_scene_file "1")

    target_include_directories(render_scene_file PRIVATE ${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

    target_link_libraries(render_scene_file MlibScene)
endif()
