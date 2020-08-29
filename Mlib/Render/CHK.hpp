#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Mlib {

void assert_no_opengl_error(const char* position, bool werror);
void assert_no_glfw_error(const char* position, bool werror);

// From: https://www.khronos.org/opengl/wiki/Example/GLSL_Shader_Compile_Error_Testing
void checked_glCompileShader(GLuint shader);
GLint checked_glGetUniformLocation(GLuint program, const GLchar *name);

#define CHK(a) a; ::Mlib::assert_no_opengl_error(#a, true)
#define WARN(a) a; ::Mlib::assert_no_opengl_error(#a, false)

#define GLFW_CHK(a) a; ::Mlib::assert_no_glfw_error(#a, true)
#define GLFW_WARN(a) a; ::Mlib::assert_no_glfw_error(#a, false)

}
