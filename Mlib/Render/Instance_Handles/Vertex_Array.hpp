#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <list>

namespace Mlib {

enum class DeallocationMode;
class IArrayBuffer;

class VertexArray {
    VertexArray(const VertexArray&) = delete;
    VertexArray& operator = (const VertexArray&) = delete;

public:
    VertexArray();
    ~VertexArray();
    void add_array_buffer(IArrayBuffer& array_buffer);
    bool initialized() const;
    void initialize();
    bool copy_in_progress() const;
    void wait() const;
    void update();
    void bind() const;
    std::list<IArrayBuffer*> array_buffers_;
    void deallocate(DeallocationMode mode);

private:
    GLuint vertex_array_;
    DeallocationToken deallocation_token_;
};

}
