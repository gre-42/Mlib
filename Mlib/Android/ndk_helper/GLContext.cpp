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
// GLContext.cpp
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// includes
//--------------------------------------------------------------------------------
#include "GLContext.h"

#include <cstring>
#include <unistd.h>

// From: https://stackoverflow.com/questions/38127022/is-there-a-standard-way-to-query-egl-error-string
static std::string eglErrorString(EGLint error)
{
  switch(error)
  {
    case EGL_SUCCESS: return "No error";
    case EGL_NOT_INITIALIZED: return "EGL not initialized or failed to initialize";
    case EGL_BAD_ACCESS: return "Resource inaccessible";
    case EGL_BAD_ALLOC: return "Cannot allocate resources";
    case EGL_BAD_ATTRIBUTE: return "Unrecognized attribute or attribute value";
    case EGL_BAD_CONTEXT: return "Invalid EGL context";
    case EGL_BAD_CONFIG: return "Invalid EGL frame buffer configuration";
    case EGL_BAD_CURRENT_SURFACE: return "Current surface is no longer valid";
    case EGL_BAD_DISPLAY: return "Invalid EGL display";
    case EGL_BAD_SURFACE: return "Invalid surface";
    case EGL_BAD_MATCH: return "Inconsistent arguments";
    case EGL_BAD_PARAMETER: return "Invalid argument";
    case EGL_BAD_NATIVE_PIXMAP: return "Invalid native pixmap";
    case EGL_BAD_NATIVE_WINDOW: return "Invalid native window";
    case EGL_CONTEXT_LOST: return "Context lost";
    default: return "Unknown error " + std::to_string(error);
  }
}

[[ noreturn ]] static void verbose_abort(const std::string& message) {
  LOGE("Aborting: %s", message.c_str());
  std::abort();
}

namespace ndk_helper {

//--------------------------------------------------------------------------------
// eGLContext
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// Ctor
//--------------------------------------------------------------------------------
GLContext::GLContext()
    : window_(nullptr),
      display_(EGL_NO_DISPLAY),
      surface_(EGL_NO_SURFACE),
      context_(EGL_NO_CONTEXT),
      screen_width_(0),
      screen_height_(0),
      gles_initialized_(false),
      egl_context_initialized_(false) {}

void GLContext::InitGLES() {
  if (gles_initialized_) return;
  //
  // Initialize OpenGL ES 3 if available
  //
  const char* versionStr = (const char*)glGetString(GL_VERSION);
  if (!strstr(versionStr, "OpenGL ES 3.")) {
    verbose_abort("Unsupported OpenGL version");
  }

  gles_initialized_ = true;
}

//--------------------------------------------------------------------------------
// Dtor
//--------------------------------------------------------------------------------
GLContext::~GLContext() { Terminate(); }

void GLContext::Init(ANativeWindow* window) {
  if (egl_context_initialized_) return;

  //
  // Initialize EGL
  //
  window_ = window;
  InitEGLSurface();
  InitEGLContext();
  InitGLES();

  egl_context_initialized_ = true;
}

void GLContext::InitEGLSurface() {
  if (window_ == nullptr) {
    verbose_abort("window is null in InitEGLSurface");
  }
  display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (display_ == EGL_NO_DISPLAY) {
    verbose_abort("eglGetDisplay failed: " + eglErrorString(eglGetError()));
  }
  if (eglInitialize(display_, nullptr, nullptr) == EGL_FALSE) {
    verbose_abort("eglInitialized failed: " + eglErrorString(eglGetError()));
  }

  /*
   * Here specify the attributes of the desired configuration.
   * Below, we select an EGLConfig with at least 8 bits per color
   * component compatible with on-screen windows
   */
  EGLint num_configs;
  {
    const EGLint attribs[] = {EGL_RENDERABLE_TYPE,
                              EGL_OPENGL_ES3_BIT,  // Request opengl ES3
                              EGL_SURFACE_TYPE,
                              EGL_WINDOW_BIT,
                              EGL_BLUE_SIZE,
                              8,
                              EGL_GREEN_SIZE,
                              8,
                              EGL_RED_SIZE,
                              8,
                              EGL_DEPTH_SIZE,
                              24,
                              EGL_NONE};
    color_size_ = 8;
    depth_size_ = 24;

    if (eglChooseConfig(display_, attribs, &config_, 1, &num_configs) == EGL_FALSE) {
      verbose_abort("eglChooseConfig with 24 bit depth failed: " + eglErrorString(eglGetError()));
    }
  }

  if (!num_configs) {
    LOGW("Could not select 24 bit depth config, trying 16 bit");
    // Fall back to 16bit depth buffer
    const EGLint attribs[] = {EGL_RENDERABLE_TYPE,
                              EGL_OPENGL_ES3_BIT,  // Request opengl ES3
                              EGL_SURFACE_TYPE,
                              EGL_WINDOW_BIT,
                              EGL_BLUE_SIZE,
                              8,
                              EGL_GREEN_SIZE,
                              8,
                              EGL_RED_SIZE,
                              8,
                              EGL_DEPTH_SIZE,
                              16,
                              EGL_NONE};
    if (eglChooseConfig(display_, attribs, &config_, 1, &num_configs) == EGL_FALSE) {
      verbose_abort("eglChooseConfig with 16 bits failed: " + eglErrorString(eglGetError()));
    }
    depth_size_ = 16;
  }

  if (!num_configs) {
    verbose_abort("Unable to retrieve EGL config");
  }

  surface_ = eglCreateWindowSurface(display_, config_, window_, nullptr);
  if (surface_ == EGL_NO_SURFACE) {
    verbose_abort("eglCreateWindowSurface failed: " + eglErrorString(eglGetError()));
  }
  if (eglQuerySurface(display_, surface_, EGL_WIDTH, &screen_width_) == EGL_FALSE) {
    verbose_abort("eglQuerySurface(EGL_WIDTH) failed: " + eglErrorString(eglGetError()));
  }
  if (eglQuerySurface(display_, surface_, EGL_HEIGHT, &screen_height_) == EGL_FALSE) {
    verbose_abort("eglQuerySurface(EGL_HEIGHT) failed: " + eglErrorString(eglGetError()));
  }
}

void GLContext::InitEGLContext() {
  const EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION,
                                    3,  // Request opengl ES3
                                    EGL_NONE};
  context_ = eglCreateContext(display_, config_, nullptr, context_attribs);

  if (eglMakeCurrent(display_, surface_, surface_, context_) == EGL_FALSE) {
    verbose_abort("Unable to eglMakeCurrent");
  }
}

EGLint GLContext::Swap() {
  bool b = eglSwapBuffers(display_, surface_);
  if (!b) {
    EGLint err = eglGetError();
    if (err == EGL_BAD_SURFACE) {
      // Recreate surface
      InitEGLSurface();
      return EGL_SUCCESS;  // Still consider glContext is valid
    } else if (err == EGL_CONTEXT_LOST || err == EGL_BAD_CONTEXT) {
      // Context has been lost!!
      Terminate();
      InitEGLContext();
    }
    return err;
  }
  return EGL_SUCCESS;
}

void GLContext::Terminate() {
  if (display_ != EGL_NO_DISPLAY) {
    eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (context_ != EGL_NO_CONTEXT) {
      eglDestroyContext(display_, context_);
    }

    if (surface_ != EGL_NO_SURFACE) {
      eglDestroySurface(display_, surface_);
    }
    eglTerminate(display_);
  }

  display_ = EGL_NO_DISPLAY;
  context_ = EGL_NO_CONTEXT;
  surface_ = EGL_NO_SURFACE;
  window_ = nullptr;
}

void GLContext::Resume(ANativeWindow* window) {
  if (!egl_context_initialized_) {
    Init(window);
    return;
  }

  // Create surface
  window_ = window;
  surface_ = eglCreateWindowSurface(display_, config_, window_, nullptr);

  if (eglMakeCurrent(display_, surface_, surface_, context_) == EGL_FALSE) {
    verbose_abort("eglMakeCurrent failed: " + eglErrorString(eglGetError()));
  }
}

void GLContext::Suspend() {
  if (surface_ != EGL_NO_SURFACE) {
    eglDestroySurface(display_, surface_);
    surface_ = EGL_NO_SURFACE;
  }
}

bool GLContext::IsSuspended() const {
  return (surface_ == EGL_NO_SURFACE);
}

void GLContext::Invalidate() {
  Terminate();

  egl_context_initialized_ = false;
}

bool GLContext::IsInitialized() const {
  return egl_context_initialized_;
}

}  // namespace ndkHelper
