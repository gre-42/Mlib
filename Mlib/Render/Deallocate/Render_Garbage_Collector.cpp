#include "Render_Garbage_Collector.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <list>
#include <mutex>

using namespace Mlib;

void GcBacklog::operator () (GLuint handle) {
    std::scoped_lock lock{mutex};
    handles.push_back(handle);
}

void GcBacklog::clear(const std::function<void(GLuint)>& deallocator) {
    std::scoped_lock lock{mutex};
    while (!handles.empty()) {
        deallocator(handles.front());
        handles.pop_front();
    }
}

GcBacklog Mlib::render_gc_append_to_frame_buffers;
GcBacklog Mlib::render_gc_append_to_textures;
GcBacklog Mlib::render_gc_append_to_render_buffers;

GcBacklog Mlib::render_gc_append_to_shaders;
GcBacklog Mlib::render_gc_append_to_programs;

GcBacklog Mlib::render_gc_append_to_vertex_arrays;
GcBacklog Mlib::render_gc_append_to_buffers;

void Mlib::execute_render_gc() {
    if (ContextQuery::is_initialized()) {
        render_gc_append_to_frame_buffers.clear([](GLuint handle){ABORT(glDeleteFramebuffers(1, &handle));});
        render_gc_append_to_textures.clear([](GLuint handle){ABORT(glDeleteTextures(1, &handle));});
        render_gc_append_to_render_buffers.clear([](GLuint handle){ABORT(glDeleteRenderbuffers(1, &handle));});
        
        render_gc_append_to_shaders.clear([](GLuint handle){ABORT(glDeleteShader(handle));});
        render_gc_append_to_programs.clear([](GLuint handle){ABORT(glDeleteProgram(handle));});

        render_gc_append_to_vertex_arrays.clear([](GLuint handle){ABORT(glDeleteVertexArrays(1, &handle));});
        render_gc_append_to_buffers.clear([](GLuint handle){ABORT(glDeleteBuffers(1, &handle));});
    }
}
