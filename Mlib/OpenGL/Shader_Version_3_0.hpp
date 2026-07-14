#pragma once

#if defined (__ANDROID__)
#define SHADER_VER "#version 300 es\n"
#define FRAGMENT_PRECISION "precision mediump float;\nprecision mediump int;\n"
#define SHADER_FIXES ""

#elif defined (__EMSCRIPTEN__)
#define SHADER_VER "#version 300 es\n"
#define FRAGMENT_PRECISION "precision mediump float;\nprecision mediump int;\n"
// Fix for Firefox and NVIDIA: Instruct the translated desktop GLSL to allow uintBitsToFloat
#define SHADER_FIXES "#extension GL_ARB_gpu_shader5 : enable\n"

#else
#define SHADER_VER "#version 460 core\n"
#define FRAGMENT_PRECISION ""
#define SHADER_FIXES ""
#endif
