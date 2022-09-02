#include "Render_Program.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Garbage_Collector.hpp>
#include <stdexcept>

using namespace Mlib;

RenderProgram::~RenderProgram() {
    // TODO: Suppress warning "Error: The GLFW library is not initialized"
    if (glfwGetCurrentContext() != nullptr) {
        deallocate();
    } else {
        gc_deallocate();
    }
}

bool RenderProgram::allocated() const {
    return vertex_shader != (GLuint)-1;
}

void RenderProgram::allocate(const char * vertex_shader_text, const char * fragment_shader_text) {
    if (allocated()) {
        throw std::runtime_error("Multiple calls to RenderProgram::generate");
    }
    CHK(vertex_shader = glCreateShader(GL_VERTEX_SHADER));
    CHK(glShaderSource(vertex_shader, 1, &vertex_shader_text, nullptr));
    checked_glCompileShader(vertex_shader);

    CHK(fragment_shader = glCreateShader(GL_FRAGMENT_SHADER));
    CHK(glShaderSource(fragment_shader, 1, &fragment_shader_text, nullptr));
    checked_glCompileShader(fragment_shader);

    CHK(program = glCreateProgram());
    CHK(glAttachShader(program, vertex_shader));
    CHK(glAttachShader(program, fragment_shader));
    CHK(glLinkProgram(program));
}

void RenderProgram::deallocate() {
    if (vertex_shader != (GLuint)-1) {
        WARN(glDeleteShader(vertex_shader));
    }
    if (fragment_shader != (GLuint)-1) {
        WARN(glDeleteShader(fragment_shader));
    }
    if (program != (GLuint)-1) {
        WARN(glDeleteProgram(program));
    }
}

void RenderProgram::gc_deallocate() {
    if (vertex_shader != (GLuint)-1) {
        render_gc_append_to_shaders(vertex_shader);
    }
    if (fragment_shader != (GLuint)-1) {
        render_gc_append_to_shaders(fragment_shader);
    }
    if (program != (GLuint)-1) {
        render_gc_append_to_programs(program);
    }
}
