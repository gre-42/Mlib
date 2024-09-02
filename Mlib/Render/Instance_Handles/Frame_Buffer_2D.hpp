#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Render/Instance_Handles/IFrame_Buffer.hpp>

namespace Mlib {

// This class is a simpler version of "FrameBufferStorage",
// but it has a mandatory "level" attribute and does not
// allocate the color texture itself.
class FrameBufferStorage2D: public IFrameBuffer {
    FrameBufferStorage2D(const FrameBufferStorage2D&) = delete;
    FrameBufferStorage2D& operator = (const FrameBufferStorage2D&) = delete;
public:
    explicit FrameBufferStorage2D(GLuint texture_color, GLint level);
    ~FrameBufferStorage2D();
    virtual bool is_configured() const override;
    virtual void bind(SourceLocation loc) override;
    virtual void unbind(SourceLocation loc) override;
private:
    void allocate(GLuint texture_color, GLint level);
    void deallocate();
    GLuint frame_buffer_ = (GLuint)-1;
    DeallocationToken deallocation_token_;
};

}
