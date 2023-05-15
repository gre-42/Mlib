#include "Render_Try_Delete.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>

using namespace Mlib;

void Mlib::try_delete_texture(GLuint& handle) {
    if (ContextQuery::is_initialized()) {
        ABORT(glDeleteTextures(1, &handle));
    } else {
        render_gc_append_to_textures(handle);
    }
    handle = (GLuint)-1;
}

void Mlib::try_delete_buffer(GLuint& handle) {
    if (ContextQuery::is_initialized()) {
        ABORT(glDeleteBuffers(1, &handle));
    } else {
        render_gc_append_to_buffers(handle);
    }
    handle = (GLuint)-1;
}
