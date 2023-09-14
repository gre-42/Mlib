#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Render/Instance_Handles/Buffer_Background_Copy.hpp>

namespace Mlib {

class VertexArray {
    VertexArray(const VertexArray&) = delete;
    VertexArray& operator = (const VertexArray&) = delete;
public:
    VertexArray();
    ~VertexArray();
    bool initialized() const;
    void initialize();
    GLuint vertex_array() const;
    void wait() const;
    BufferBackgroundCopy vertex_buffer;
    BufferBackgroundCopy bone_weight_buffer;
    BufferBackgroundCopy texture_layer_buffer;
    BufferBackgroundCopy interior_mapping_buffer;
    void deallocate();
    void gc_deallocate();
private:
    GLuint vertex_array_;
    DeallocationToken deallocation_token_;
};

}
