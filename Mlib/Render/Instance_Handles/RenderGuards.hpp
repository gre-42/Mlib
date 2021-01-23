#pragma once
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>

namespace Mlib {

struct FrameBufferMsaa;

class RenderToFrameBufferGuard {
    friend class RenderToScreenGuard;
public:
    explicit RenderToFrameBufferGuard(const FrameBufferMsaa& fb);
    ~RenderToFrameBufferGuard();
private:
    const FrameBufferMsaa* last_frame_buffer_;
    static const FrameBufferMsaa* first_frame_buffer_;
    static bool is_empty_;
    static size_t stack_size_;
};

class RenderToScreenGuard {
public:
    RenderToScreenGuard();
    ~RenderToScreenGuard();
};

}
