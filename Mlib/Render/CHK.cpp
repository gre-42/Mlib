#include "CHK.hpp"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

void Mlib::assert_no_opengl_error(const char* position, bool werror) {
    GLenum code = glGetError();
    if (code != GL_NO_ERROR) {
        std::string descr = std::to_string(code);
        if (code == GL_INVALID_VALUE) {
            descr += " (invalid value)";
        } else if (code == GL_INVALID_OPERATION) {
            descr += " (invalid operation)";
        }
        std::string msg = "OpenGL error at line \"" + std::string(position) + "\": " + descr;
        if (werror) {
            throw std::runtime_error(msg);
        } else {
            std::cerr << "WARNING: " << msg << std::endl;
        }
    }
}

void Mlib::assert_no_glfw_error(const char* position, bool werror) {
    const char* description;
    int code = glfwGetError(&description);
    if (code != GLFW_NO_ERROR) {
        std::string msg = "OpenGL error at line \"" + std::string(position) + "\": " + std::string(description);
        if (werror) {
            throw std::runtime_error(msg);
        } else {
            std::cerr << "WARNING: " << msg << std::endl;
        }
    }
}

GLint Mlib::checked_glGetUniformLocation(GLuint program, const GLchar *name) {
    CHK(GLint result = glGetUniformLocation(program, name));
    if (result == -1) {
        throw std::runtime_error("Unknown variable: " + std::string(name));
    }
    return result;
}

// From: https://www.khronos.org/opengl/wiki/Example/GLSL_Shader_Compile_Error_Testing
void Mlib::checked_glCompileShader(GLuint shader) {
    CHK(glCompileShader(shader));

    GLint isCompiled = 0;
    CHK(glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled));
    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        CHK(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength));

        // The maxLength includes the NULL character
        std::vector<GLchar> errorLog(maxLength);
        CHK(glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]));

        // Provide the infolog in whatever manor you deem best.
        throw std::runtime_error(std::string(errorLog.begin(), errorLog.end() - 1));

        // Exit with failure.
        // glDeleteShader(shader); // Don't leak the shader.
    }
}
