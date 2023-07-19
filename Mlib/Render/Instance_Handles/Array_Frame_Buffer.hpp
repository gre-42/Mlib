#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>

namespace Mlib {

class ArrayFrameBufferStorage {
    ArrayFrameBufferStorage(const ArrayFrameBufferStorage&) = delete;
    ArrayFrameBufferStorage& operator = (const ArrayFrameBufferStorage&) = delete;
public:
    explicit ArrayFrameBufferStorage(GLuint texture_color, GLint level, GLint layer);
    ~ArrayFrameBufferStorage();
private:
    void allocate(GLuint texture_color, GLint level, GLint layer);
    void deallocate();
    void bind() const;
    void unbind() const;
    GLuint frame_buffer_ = (GLuint)-1;
    DeallocationToken deallocation_token_;
};

}
