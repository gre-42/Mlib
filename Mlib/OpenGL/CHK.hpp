#pragma once
#include <Mlib/OpenGL/Any_Gl.hpp>

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

enum class PrintGlCalls {
    DISABLED = 0,
    ENABLED = 1
};

enum class PrintRenderedMaterials {
    DISABLED = 0,
    ENABLED = 1
};

}

#ifndef __ANDROID__

namespace Mlib {

enum class PrintGlfwCalls {
    DISABLED = 0,
    ENABLED = 1
};

void check_glfw_errors(CheckErrors check);
void print_glfw_calls(PrintGlfwCalls print);

void assert_no_glfw_error(const char* position, FailureBehavior failure_behavior);
void log_glfw_call(const char* position);

#define GLFW_WARN(a) do {::Mlib::log_glfw_call(#a); a; ::Mlib::assert_no_glfw_error(#a, ::Mlib::FailureBehavior::WARN);} while (false)
#define GLFW_CHK(a) do {::Mlib::log_glfw_call(#a); a; ::Mlib::assert_no_glfw_error(#a, ::Mlib::FailureBehavior::THROW);} while (false)
#define GLFW_ABORT(a) do {::Mlib::log_glfw_call(#a); a; ::Mlib::assert_no_glfw_error(#a, ::Mlib::FailureBehavior::ABORT);} while (false)

#define GLFW_WARN_X(a) [&]{::Mlib::log_glfw_call(#a); auto res = a; ::Mlib::assert_no_glfw_error(#a, ::Mlib::FailureBehavior::WARN); return res;}()
#define GLFW_CHK_X(a) [&]{::Mlib::log_glfw_call(#a); auto res = a; ::Mlib::assert_no_glfw_error(#a, ::Mlib::FailureBehavior::THROW); return res;}()
#define GLFW_ABORT_X(a) [&]{::Mlib::log_glfw_call(#a); auto res = a; ::Mlib::assert_no_glfw_error(#a, ::Mlib::FailureBehavior::ABORT); return res;}()

}

#endif

namespace Mlib {

void check_gl_errors(CheckErrors check);
void print_gl_calls(PrintGlCalls print);
void print_rendered_materials(PrintRenderedMaterials print);
bool print_rendered_materials();

void assert_no_opengl_error(const char* position, FailureBehavior failure_behavior);
void log_opengl_call(const char* position);

// From: https://www.khronos.org/opengl/wiki/Example/GLSL_Shader_Compile_Error_Testing
void checked_glCompileShader(GLuint shader);
void checked_glLinkProgram(GLuint program);
GLint checked_glGetUniformLocation(GLuint program, const GLchar *name);

#define WARN(a) do {::Mlib::log_opengl_call(#a); a; ::Mlib::assert_no_opengl_error(#a, ::Mlib::FailureBehavior::WARN);} while (false)
#define CHK(a) do {::Mlib::log_opengl_call(#a); a; ::Mlib::assert_no_opengl_error(#a, ::Mlib::FailureBehavior::THROW);} while (false)
#define ABORT(a) do {::Mlib::log_opengl_call(#a); a; ::Mlib::assert_no_opengl_error(#a, ::Mlib::FailureBehavior::ABORT);} while (false)

#define WARN_X(a) [&]{::Mlib::log_opengl_call(#a); auto res = a; ::Mlib::assert_no_opengl_error(#a, ::Mlib::FailureBehavior::WARN); return res;}()
#define CHK_X(a) [&]{::Mlib::log_opengl_call(#a); auto res = a; ::Mlib::assert_no_opengl_error(#a, ::Mlib::FailureBehavior::THROW); return res;}()
#define ABORT_X(a) [&]{::Mlib::log_opengl_call(#a); auto res = a; ::Mlib::assert_no_opengl_error(#a, ::Mlib::FailureBehavior::ABORT); return res;}()

}
