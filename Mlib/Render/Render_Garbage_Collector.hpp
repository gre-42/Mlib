#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <list>

namespace Mlib {

extern std::list<GLuint> gc_frame_buffers;
extern std::list<GLuint> gc_texture_color_buffers;
extern std::list<GLuint> gc_texture_depth_buffers;
extern std::list<GLuint> gc_render_buffers;

void execute_gc_render();

}
