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
private:
    void allocate(GLuint texture_color, GLint level, GLint layer);
    void deallocate();
    bool is_configured() const override;
    void bind() override;
    void unbind() override;
    GLuint frame_buffer_ = (GLuint)-1;
    DeallocationToken deallocation_token_;
};

}
