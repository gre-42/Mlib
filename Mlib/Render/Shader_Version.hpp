#pragma once

#ifdef __ANDROID__
#define SHADER_VER "#version 310 es\n"
#else
#define SHADER_VER "#version 330 core\n"
#endif
