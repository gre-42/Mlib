#include "Render_Program.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <stdexcept>

using namespace Mlib;

RenderProgram::RenderProgram()
: deallocation_token_{render_deallocator.insert([this](){deallocate();})}
{}

RenderProgram::~RenderProgram() {
    // TODO: Suppress warning "Error: The GLFW library is not initialized"
    if (ContextQuery::is_initialized()) {
        deallocate();
    } else {
        gc_deallocate();
    }
}

bool RenderProgram::allocated() const {
    return vertex_shader != (GLuint)-1;
}

void RenderProgram::allocate(const char* vertex_shader_text, const char* fragment_shader_text) {
    if (allocated()) {
        THROW_OR_ABORT("Multiple calls to RenderProgram::allocate");
    }
    CHK(vertex_shader = glCreateShader(GL_VERTEX_SHADER));
    if (vertex_shader == 0) {
        THROW_OR_ABORT("glCreateShader(GL_VERTEX_SHADER) returned 0");
    }
    CHK(glShaderSource(vertex_shader, 1, &vertex_shader_text, nullptr));
    checked_glCompileShader(vertex_shader);

    CHK(fragment_shader = glCreateShader(GL_FRAGMENT_SHADER));
    if (fragment_shader == 0) {
        THROW_OR_ABORT("glCreateShader(GL_FRAGMENT_SHADER) returned 0");
    }
    CHK(glShaderSource(fragment_shader, 1, &fragment_shader_text, nullptr));
    checked_glCompileShader(fragment_shader);

    CHK(program = glCreateProgram());
    CHK(glAttachShader(program, vertex_shader));
    CHK(glAttachShader(program, fragment_shader));
    checked_glLinkProgram(program);
}

void RenderProgram::deallocate() {
    if (vertex_shader != (GLuint)-1) {
        ABORT(glDeleteShader(vertex_shader));
        vertex_shader = (GLuint)-1;
    }
    if (fragment_shader != (GLuint)-1) {
        ABORT(glDeleteShader(fragment_shader));
        fragment_shader = (GLuint)-1;
    }
    if (program != (GLuint)-1) {
        ABORT(glDeleteProgram(program));
        program = (GLuint)-1;
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
