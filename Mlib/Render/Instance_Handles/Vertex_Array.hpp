#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Render/Instance_Handles/Buffer_Background_Copy.hpp>

namespace Mlib {

enum class DeallocationMode;

class VertexArray {
    VertexArray(const VertexArray&) = delete;
    VertexArray& operator = (const VertexArray&) = delete;

public:
    VertexArray();
    ~VertexArray();
    bool initialized() const;
    void initialize();
    bool copy_in_progress() const;
    GLuint vertex_array() const;
    void wait() const;
    BufferBackgroundCopy vertex_buffer;
    BufferBackgroundCopy bone_weight_buffer;
    BufferBackgroundCopy texture_layer_buffer;
    BufferBackgroundCopy interior_mapping_buffer;
    void deallocate(DeallocationMode mode);

private:
    GLuint vertex_array_;
    DeallocationToken deallocation_token_;
};

}
