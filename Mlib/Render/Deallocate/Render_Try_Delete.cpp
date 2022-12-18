#include "Render_Try_Delete.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Obtainer.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>

using namespace Mlib;

void Mlib::try_delete_texture(GLuint& handle) {
    if (ContextObtainer::is_initialized()) {
        WARN(glDeleteTextures(1, &handle));
    } else {
        render_gc_append_to_textures(handle);
    }
    handle = (GLuint)-1;
}
