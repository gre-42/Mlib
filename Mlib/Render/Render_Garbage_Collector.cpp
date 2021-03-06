#include "Render_Garbage_Collector.hpp"
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

std::list<GLuint> Mlib::gc_frame_buffers;
std::list<GLuint> Mlib::gc_textures;
std::list<GLuint> Mlib::gc_render_buffers;

std::list<GLuint> Mlib::gc_shaders;
std::list<GLuint> Mlib::gc_programs;

std::list<GLuint> Mlib::gc_vertex_arrays;
std::list<GLuint> Mlib::gc_buffers;

void Mlib::execute_gc_render() {
    if (glfwGetCurrentContext() != nullptr) {
        while (!gc_frame_buffers.empty()) {
            WARN(glDeleteFramebuffers(1, &gc_frame_buffers.front()));
            gc_frame_buffers.pop_front();
        }
        while (!gc_textures.empty()) {
            WARN(glDeleteTextures(1, &gc_textures.front()));
            gc_textures.pop_front();
        }
        while (!gc_render_buffers.empty()) {
            WARN(glDeleteRenderbuffers(1, &gc_render_buffers.front()));
            gc_render_buffers.pop_front();
        }

        while (!gc_shaders.empty()) {
            WARN(glDeleteShader(gc_shaders.front()));
            gc_shaders.pop_front();
        }
        while (!gc_programs.empty()) {
            WARN(glDeleteProgram(gc_programs.front()));
            gc_programs.pop_front();
        }

        while (!gc_vertex_arrays.empty()) {
            WARN(glDeleteVertexArrays(1, &gc_vertex_arrays.front()));
            gc_vertex_arrays.pop_front();
        }
        while (!gc_buffers.empty()) {
            WARN(glDeleteBuffers(1, &gc_buffers.front()));
            gc_buffers.pop_front();
        }
    }
}
