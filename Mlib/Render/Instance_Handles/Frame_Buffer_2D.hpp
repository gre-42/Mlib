#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>

namespace Mlib {

class FrameBufferStorage2D {
    FrameBufferStorage2D(const FrameBufferStorage2D&) = delete;
    FrameBufferStorage2D& operator = (const FrameBufferStorage2D&) = delete;
public:
    explicit FrameBufferStorage2D(GLuint texture_color, GLint level);
    ~FrameBufferStorage2D();
private:
    void allocate(GLuint texture_color, GLint level);
    void deallocate();
    void bind() const;
    void unbind() const;
    GLuint frame_buffer_ = (GLuint)-1;
    DeallocationToken deallocation_token_;
};

}
