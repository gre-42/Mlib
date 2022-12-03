#pragma once
#ifdef __ANDROID__
#include <GLES3/gl3.h>
#else
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#include <functional>
#include <list>
#include <mutex>

namespace Mlib {

struct GcBacklog {
    std::mutex mutex;
    std::list<GLuint> handles;
    void operator () (GLuint handle);
    void clear(const std::function<void(GLuint)>& deallocator);
};

extern GcBacklog render_gc_append_to_frame_buffers;
extern GcBacklog render_gc_append_to_textures;
extern GcBacklog render_gc_append_to_render_buffers;

extern GcBacklog render_gc_append_to_shaders;
extern GcBacklog render_gc_append_to_programs;

extern GcBacklog render_gc_append_to_vertex_arrays;
extern GcBacklog render_gc_append_to_buffers;

/**
 * Garbage collector for abandoned rendering resources.
 *
 * Required when deleting objects in a different thread.
 */
void execute_render_gc();

}
