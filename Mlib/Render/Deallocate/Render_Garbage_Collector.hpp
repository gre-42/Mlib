#pragma once
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Threads/Atomic_Mutex.hpp>
#include <functional>
#include <list>
#include <mutex>

namespace Mlib {

struct GcBacklog {
    AtomicMutex mutex;
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
