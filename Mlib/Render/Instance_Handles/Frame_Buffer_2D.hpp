#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Render/Instance_Handles/IFrame_Buffer.hpp>

namespace Mlib {

class FrameBufferStorage2D: public IFrameBuffer {
    FrameBufferStorage2D(const FrameBufferStorage2D&) = delete;
    FrameBufferStorage2D& operator = (const FrameBufferStorage2D&) = delete;
public:
    explicit FrameBufferStorage2D(GLuint texture_color, GLint level);
    ~FrameBufferStorage2D();
private:
    void allocate(GLuint texture_color, GLint level);
    void deallocate();
    bool is_configured() const override;
    void bind() const override;
    void unbind() const override;
    GLuint frame_buffer_ = (GLuint)-1;
    DeallocationToken deallocation_token_;
};

}
