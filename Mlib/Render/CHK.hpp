#pragma once
#include <Mlib/Log.hpp>
#include <Mlib/Render/Any_Gl.hpp>

namespace Mlib {

enum class FailureBehavior {
    WARN,
    THROW,
    ABORT
};

enum class CheckErrors {
    DISABLED = 0,
    ENABLED = 1
};

}

#ifndef __ANDROID__

namespace Mlib {

void check_glfw_errors(CheckErrors check);

void assert_no_glfw_error(const char* position, FailureBehavior failure_behavior);

#define GLFW_WARN(a) a; ::Mlib::assert_no_glfw_error(#a, ::Mlib::FailureBehavior::WARN)
#define GLFW_CHK(a) a; ::Mlib::assert_no_glfw_error(#a, ::Mlib::FailureBehavior::THROW)
#define GLFW_ABORT(a) a; ::Mlib::assert_no_glfw_error(#a, ::Mlib::FailureBehavior::ABORT)

}

#endif

namespace Mlib {

void check_gl_errors(CheckErrors check);

void assert_no_opengl_error(const char* position, FailureBehavior failure_behavior);

// From: https://www.khronos.org/opengl/wiki/Example/GLSL_Shader_Compile_Error_Testing
void checked_glCompileShader(GLuint shader);
void checked_glLinkProgram(GLuint program);
GLint checked_glGetUniformLocation(GLuint program, const GLchar *name);

#define WARN(a) LOG_INFO(#a); a; ::Mlib::assert_no_opengl_error(#a, ::Mlib::FailureBehavior::WARN)
#define CHK(a) LOG_INFO(#a); a; ::Mlib::assert_no_opengl_error(#a, ::Mlib::FailureBehavior::THROW)
#define ABORT(a) LOG_INFO(#a); a; ::Mlib::assert_no_opengl_error(#a, ::Mlib::FailureBehavior::ABORT)

}
