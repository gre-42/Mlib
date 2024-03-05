#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>

namespace Mlib {

enum class DeallocationMode;
class IArrayBuffer;

class VertexArray {
    VertexArray(const VertexArray&) = delete;
    VertexArray& operator = (const VertexArray&) = delete;

public:
    VertexArray(
        IArrayBuffer& vertex_buffer,
        IArrayBuffer& bone_weight_buffer,
        IArrayBuffer& texture_layer_buffer,
        IArrayBuffer& interior_mapping_buffer);
    ~VertexArray();
    bool initialized() const;
    void initialize();
    bool copy_in_progress() const;
    void wait() const;
    void update();
    void bind() const;
    IArrayBuffer& vertex_buffer;
    IArrayBuffer& bone_weight_buffer;
    IArrayBuffer& texture_layer_buffer;
    IArrayBuffer& interior_mapping_buffer;
    void deallocate(DeallocationMode mode);

private:
    GLuint vertex_array_;
    DeallocationToken deallocation_token_;
};

}
