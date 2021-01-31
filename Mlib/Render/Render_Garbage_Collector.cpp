#include "Render_Garbage_Collector.hpp"
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

std::list<GLuint> Mlib::gc_frame_buffers;
std::list<GLuint> Mlib::gc_textures_;
std::list<GLuint> Mlib::gc_render_buffers;

void Mlib::execute_gc_render() {
    if (glfwGetCurrentContext() == nullptr) {
        while(!gc_frame_buffers.empty()) {
            WARN(glDeleteFramebuffers(1, &gc_frame_buffers.front()));
            gc_frame_buffers.pop_front();
        }
        while(!gc_textures_.empty()) {
            WARN(glDeleteTextures(1, &gc_textures_.front()));
            gc_textures_.pop_front();
        }
        while(!gc_render_buffers.empty()) {
            WARN(glDeleteRenderbuffers(1, &gc_render_buffers.front()));
            gc_render_buffers.pop_front();
        }
    }
}
