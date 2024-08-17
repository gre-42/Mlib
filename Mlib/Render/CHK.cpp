#include "CHK.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Mlib;

static bool CHECK_GL_ERRORS = false;

void Mlib::check_gl_errors(CheckErrors check) {
    CHECK_GL_ERRORS = (check == CheckErrors::ENABLED);
}

void Mlib::assert_no_opengl_error(const char* position, FailureBehavior failure_behavior) {
    if (!CHECK_GL_ERRORS) {
        return;
    }
    GLenum code = glGetError();
    if (code != GL_NO_ERROR) {
        std::string descr = std::to_string(code);
        if (code == GL_INVALID_VALUE) {
            descr += " (invalid value)";
        } else if (code == GL_INVALID_OPERATION) {
            descr += " (invalid operation)";
        } else if (code == GL_INVALID_ENUM) {
            descr += " (invalid enum)";
        }
        std::string msg = "OpenGL error at line \"" + std::string(position) + "\": " + descr;
        if (failure_behavior == FailureBehavior::WARN) {
            lwarn() << msg;
        } else if (failure_behavior == FailureBehavior::THROW) {
            THROW_OR_ABORT(msg);
        } else if (failure_behavior == FailureBehavior::ABORT) {
            verbose_abort(msg);
        } else {
            verbose_abort("Unknown OpenGL error behavior: " + std::to_string(int(failure_behavior)));
        }
    }
}

#ifndef __ANDROID__

static bool CHECK_GLFW_ERRORS = true;

void Mlib::check_glfw_errors(CheckErrors check) {
    CHECK_GLFW_ERRORS = (check == CheckErrors::ENABLED);
}

void Mlib::assert_no_glfw_error(const char* position, FailureBehavior failure_behavior) {
    if (!CHECK_GLFW_ERRORS) {
        return;
    }
    const char* description;
    int code = glfwGetError(&description);
    if (code != GLFW_NO_ERROR) {
        std::string msg = "OpenGL error at line \"" + std::string(position) + "\": " + std::string(description);
        if (failure_behavior == FailureBehavior::WARN) {
            lwarn() << msg;
        } else if (failure_behavior == FailureBehavior::THROW) {
            THROW_OR_ABORT(msg);
        } else if (failure_behavior == FailureBehavior::ABORT) {
            verbose_abort(msg);
        } else {
            verbose_abort("Unknown GLFW error behavior: " + std::to_string(int(failure_behavior)));
        }
    }
}
#endif

GLint Mlib::checked_glGetUniformLocation(GLuint program, const GLchar *name) {
    CHK(GLint result = glGetUniformLocation(program, name));
    if (result == -1) {
        THROW_OR_ABORT("Unknown variable: " + std::string(name));
    }
    return result;
}

// From: https://www.khronos.org/opengl/wiki/Example/GLSL_Shader_Compile_Error_Testing
void Mlib::checked_glCompileShader(GLuint shader) {
    CHK(glCompileShader(shader));

    GLint is_compiled = 0;
    CHK(glGetShaderiv(shader, GL_COMPILE_STATUS, &is_compiled));
    if (is_compiled == GL_FALSE) {
        GLint max_length = 0;
        CHK(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length));

        if (max_length <= 0) {
            THROW_OR_ABORT("Shader compile log length is not positive");
        }

        // The max_length includes the NULL character
        std::vector<GLchar> error_log((size_t)max_length);
        CHK(glGetShaderInfoLog(shader, max_length, &max_length, error_log.data()));

        if ((size_t)max_length != error_log.size() - 1) {
            THROW_OR_ABORT("Conflicting shader compile log lengths");
        }
        // The shader is deleted in a destructor elsewhere.
        // glDeleteShader(shader);

        // Provide the infolog in whatever manner you deem best.
        THROW_OR_ABORT(std::string(error_log.begin(), error_log.end() - 1));
    }
}

// From: https://www.khronos.org/opengl/wiki/Example/GLSL_Shader_Compile_Error_Testing
void Mlib::checked_glLinkProgram(GLuint program) {
    CHK(glLinkProgram(program));

    GLint is_linked = 0;
    CHK(glGetProgramiv(program, GL_LINK_STATUS, &is_linked));
    if (is_linked == GL_FALSE) {
        GLint max_length = 0;
        CHK(glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length));

        if (max_length <= 0) {
            THROW_OR_ABORT("Shader program link log length is not positive");
        }

        // The max_length includes the NULL character
        std::vector<GLchar> error_log((size_t)max_length);
        CHK(glGetProgramInfoLog(program, max_length, &max_length, error_log.data()));

        if ((size_t)max_length != error_log.size() - 1) {
            THROW_OR_ABORT("Conflicting shader program link log lengths");
        }
        // The program is deleted in a destructor elsewhere.
        // glDeleteProgram(program);

        // Provide the infolog in whatever manner you deem best.
        THROW_OR_ABORT(std::string(error_log.begin(), error_log.end() - 1));
    }
}
