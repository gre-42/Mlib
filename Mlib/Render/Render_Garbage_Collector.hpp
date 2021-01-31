#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <list>

namespace Mlib {

extern std::list<GLuint> gc_frame_buffers;
extern std::list<GLuint> gc_textures_;
extern std::list<GLuint> gc_render_buffers;

/**
 * Garbage collector for abandoned rendering resources.
 *
 * Required when deleting objects in a different thread.
 */
void execute_gc_render();

}
