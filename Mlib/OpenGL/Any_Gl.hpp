#pragma once
#ifdef __ANDROID__
#include <GLES3/gl32.h>
#else
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif
