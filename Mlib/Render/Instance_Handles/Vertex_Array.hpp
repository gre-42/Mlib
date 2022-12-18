#pragma once
#ifdef __ANDROID__
#include <GLES3/gl32.h>
#else
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#include <Mlib/Deallocation_Token.hpp>

namespace Mlib {

class VertexArray {
    VertexArray(const VertexArray&) = delete;
    VertexArray& operator = (const VertexArray&) = delete;
public:
    VertexArray();
    ~VertexArray();
    GLuint vertex_array = (GLuint)-1;
    GLuint vertex_buffer = (GLuint)-1;
    GLuint position_buffer = (GLuint)-1;
    GLuint bone_weight_buffer = (GLuint)-1;
    GLuint interior_mapping_buffer = (GLuint)-1;
    GLuint billboard_id_buffer = (GLuint)-1;
    void deallocate();
    void gc_deallocate();
private:
    DeallocationToken deallocation_token_;
};

}
