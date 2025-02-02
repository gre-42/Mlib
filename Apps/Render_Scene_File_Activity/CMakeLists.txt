include(../../CMakeCommands.cmake)

set(mlib_excluded_files "")

my_add_shared_library(RenderSceneFileActivity "1" "" "${mlib_excluded_files}")

target_include_directories(RenderSceneFileActivity PRIVATE ${Mlib_INCLUDE_DIR} ${glfw3_INCLUDE_DIR})

target_link_libraries(RenderSceneFileActivity PRIVATE MlibScene AGameHelper)

# Export ANativeActivity_onCreate(),
# Refer to: https://github.com/android-ndk/ndk/issues/381.
set_target_properties(RenderSceneFileActivity
        PROPERTIES  LINK_FLAGS "-u ANativeActivity_onCreate")
