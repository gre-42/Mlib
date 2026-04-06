#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/OpenGL/Any_Gl.hpp>
#include <iosfwd>
#include <list>

namespace Mlib {

enum class DeallocationMode;
class IArrayBuffer;
class IGpuInstanceBuffers;

class VertexArray {
    VertexArray(const VertexArray&) = delete;
    VertexArray& operator = (const VertexArray&) = delete;

public:
    VertexArray();
    ~VertexArray();
    void add_array_buffer(IArrayBuffer& array_buffer);
    void set_instance_buffer(IGpuInstanceBuffers& instance_buffer);
    bool initialized() const;
    void initialize();
    bool copy_in_progress() const;
    void wait() const;
    void update();
    void bind() const;
    void deallocate(DeallocationMode mode);
    void print_stats(std::ostream& ostr) const;

private:
    std::list<IArrayBuffer*> array_buffers_;
    IGpuInstanceBuffers* instance_buffer_;
    GLuint vertex_array_;
    DeallocationToken deallocation_token_;
};

}
