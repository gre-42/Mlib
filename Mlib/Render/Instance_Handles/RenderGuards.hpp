#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Mlib {

struct FrameBuffer;

class RenderToFrameBufferGuard {
    friend class RenderToScreenGuard;
public:
    explicit RenderToFrameBufferGuard(const FrameBuffer& fb);
    ~RenderToFrameBufferGuard();
private:
    GLuint last_frame_buffer_;
    static GLuint first_frame_buffer_;
    static bool is_empty_;
    static size_t stack_size_;
};

class RenderToScreenGuard {
public:
    RenderToScreenGuard();
    ~RenderToScreenGuard();
};

}
