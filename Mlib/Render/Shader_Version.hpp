#pragma once

#ifdef __ANDROID__
#define SHADER_VER "#version 310 es\n"
#define FRAGMENT_PRECISION "precision mediump float;\nprecision mediump int;\n"
#else
#define SHADER_VER "#version 330 core\n"
#define FRAGMENT_PRECISION ""
#endif