/*
 * Copyright 2013 The Android Open Source Project
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

//--------------------------------------------------------------------------------
// GLContext.h
//--------------------------------------------------------------------------------
#ifndef GLCONTEXT_H_
#define GLCONTEXT_H_

#include <EGL/egl.h>
#include <GLES3/gl32.h>
#include <android/log.h>

#include "JNIHelper.h"

namespace ndk_helper {

//--------------------------------------------------------------------------------
// Constants
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Class
//--------------------------------------------------------------------------------

/******************************************************************
 * OpenGL context handler
 * The class handles OpenGL and EGL context based on Android activity life cycle
 * The caller needs to call corresponding methods for each activity life cycle
 *events as it's done in sample codes.
 *
 * Also the class initializes OpenGL ES3 when the compatible driver is installed
 *in the device.
 * getGLVersion() returns 3.0~ when the device supports OpenGLES3.0
 *
 * Thread safety: OpenGL context is expecting used within dedicated single
 *thread,
 * thus GLContext class is not designed as a thread-safe
 */
class GLContext {
 private:
  // EGL configurations
  ANativeWindow* window_;
  EGLDisplay display_;
  EGLSurface surface_;
  EGLContext context_;
  EGLConfig config_;

  // Screen parameters
  int32_t color_size_;
  int32_t depth_size_;

  // Flags
  bool gles_initialized_;
  bool egl_context_initialized_;

  void InitGLES();
  void Terminate();
  void InitEGLSurface();
  void InitEGLContext();

  GLContext(GLContext const&) = delete;
  GLContext& operator = (GLContext const&) = delete;
  GLContext();
  virtual ~GLContext();

 public:
  static GLContext* GetInstance() {
    // Singleton
    static GLContext instance;

    return &instance;
  }

  void Init(ANativeWindow* window);
  void Swap();
  void Invalidate();
  bool IsInitialized() const;

  void Suspend();
  bool IsSuspended() const;
  void Resume(ANativeWindow* window);

  ANativeWindow* GetANativeWindow() const { return window_; };
  int32_t GetScreenWidth() const;
  int32_t GetScreenHeight() const;

  EGLSurface GetSurface() const { return surface_; }
};

}  // namespace ndkHelper

#endif /* GLCONTEXT_H_ */
