#include "Render_Program.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <stdexcept>

using namespace Mlib;

RenderProgram::RenderProgram()
    : deallocation_token_{ render_deallocator.insert([this]() { deallocate(); }) }
{}

RenderProgram::~RenderProgram() {
    if (ContextQuery::is_initialized()) {
        deallocate();
    } else {
        gc_deallocate();
    }
}

bool RenderProgram::allocated() const {
    return vertex_shader_ != 0;
}

void RenderProgram::allocate(const char* vertex_shader_text, const char* fragment_shader_text) {
    if (allocated()) {
        THROW_OR_ABORT("Multiple calls to RenderProgram::allocate");
    }
    CHK(vertex_shader_ = glCreateShader(GL_VERTEX_SHADER));
    if (vertex_shader_ == 0) {
        THROW_OR_ABORT("glCreateShader(GL_VERTEX_SHADER) returned 0");
    }
    CHK(glShaderSource(vertex_shader_, 1, &vertex_shader_text, nullptr));
    checked_glCompileShader(vertex_shader_);

    CHK(fragment_shader_ = glCreateShader(GL_FRAGMENT_SHADER));
    if (fragment_shader_ == 0) {
        THROW_OR_ABORT("glCreateShader(GL_FRAGMENT_SHADER) returned 0");
    }
    CHK(glShaderSource(fragment_shader_, 1, &fragment_shader_text, nullptr));
    checked_glCompileShader(fragment_shader_);

    CHK(program_ = glCreateProgram());
    CHK(glAttachShader(program_, vertex_shader_));
    CHK(glAttachShader(program_, fragment_shader_));
    checked_glLinkProgram(program_);
}

void RenderProgram::deallocate() {
    if (vertex_shader_ != 0) {
        ABORT(glDeleteShader(vertex_shader_));
        vertex_shader_ = 0;
    }
    if (fragment_shader_ != 0) {
        ABORT(glDeleteShader(fragment_shader_));
        fragment_shader_ = 0;
    }
    if (program_ != 0) {
        ABORT(glDeleteProgram(program_));
        program_ = 0;
    }
}

void RenderProgram::gc_deallocate() {
    if (vertex_shader_ != 0) {
        render_gc_append_to_shaders(vertex_shader_);
    }
    if (fragment_shader_ != 0) {
        render_gc_append_to_shaders(fragment_shader_);
    }
    if (program_ != 0) {
        render_gc_append_to_programs(program_);
    }
}

void RenderProgram::use() const {
    if (program_ == 0) {
        THROW_OR_ABORT("Render program not allocated");
    }
    CHK(glUseProgram(program_));
}

GLint RenderProgram::get_uniform_location(const char* name) const {
    return checked_glGetUniformLocation(program_, name);
}
