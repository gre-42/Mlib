#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Render/Instance_Handles/IFrame_Buffer.hpp>

namespace Mlib {

class ArrayFrameBufferStorage: public IFrameBuffer {
    ArrayFrameBufferStorage(const ArrayFrameBufferStorage&) = delete;
    ArrayFrameBufferStorage& operator = (const ArrayFrameBufferStorage&) = delete;
public:
    explicit ArrayFrameBufferStorage(GLuint texture_color, GLint level, GLint layer);
    ~ArrayFrameBufferStorage();
    virtual bool is_configured() const override;
    virtual void bind(SourceLocation loc) override;
    virtual void unbind(SourceLocation loc) override;
private:
    void allocate(GLuint texture_color, GLint level, GLint layer);
    void deallocate();
    GLuint frame_buffer_ = (GLuint)-1;
    DeallocationToken deallocation_token_;
};

}
