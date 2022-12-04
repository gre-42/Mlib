/*
 * Copyright 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// From: https://github.com/android/ndk-samples/blob/main/teapots/classic-teapot/src/main/cpp/TeapotNativeActivity.cpp

//--------------------------------------------------------------------------------
// Include files
//--------------------------------------------------------------------------------
#include <jni.h>
#include <cerrno>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <android/native_window_jni.h>

#include "NdkTestRenderer.hpp"
#include "NDKHelper.h"
#include <Mlib/Android/game_helper/AEngine.hpp>
#include <Mlib/Android/game_helper/AWindow.hpp>
#include <Mlib/Android/game_helper/ARenderWindow.hpp>

using namespace Mlib;

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(android_app* app) {
    NdkTestRenderer renderer;
    AEngine a_engine{renderer};
    ARenderWindow render_window{*app, a_engine};
    AWindow window{*app->window};
    render_window.render_loop();
}
