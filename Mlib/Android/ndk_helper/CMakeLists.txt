# build native_app_glue as a static lib
cmake_minimum_required(VERSION 3.22.1)

include(AndroidNdkModules)
android_ndk_import_module_native_app_glue()
# https://stackoverflow.com/questions/11801186/cmake-unable-to-determine-linker-language-with-c
set_target_properties(native_app_glue PROPERTIES LINKER_LANGUAGE CXX)

add_library(NdkHelper
  STATIC
    gestureDetector.cpp
    GLContext.cpp
    JNIHelper.cpp
    sensorManager.cpp
    tapCamera.cpp
    vecmath.cpp
    AUi.cpp
    AndroidApp.cpp
)
set_target_properties(NdkHelper
  PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    INTERFACE_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:NdkHelper,INCLUDE_DIRECTORIES>
)
target_include_directories(NdkHelper
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../
)

target_link_libraries(NdkHelper
  PUBLIC
    native_app_glue
    GLESv3
    EGL
    log
    android
    atomic
    MlibOs
)
