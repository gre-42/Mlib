#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>

namespace Mlib {

class VertexArray {
    VertexArray(const VertexArray&) = delete;
    VertexArray& operator = (const VertexArray&) = delete;
public:
    VertexArray();
    ~VertexArray();
    GLuint vertex_array = (GLuint)-1;
    GLuint vertex_buffer = (GLuint)-1;
    GLuint bone_weight_buffer = (GLuint)-1;
    GLuint interior_mapping_buffer = (GLuint)-1;
    void deallocate();
    void gc_deallocate();
private:
    DeallocationToken deallocation_token_;
};

}
