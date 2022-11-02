#pragma once
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>

namespace Mlib {

class FrameBuffer;

class RenderToFrameBufferGuard {
    friend class RenderToScreenGuard;
    RenderToFrameBufferGuard(const RenderToFrameBufferGuard&) = delete;
    RenderToFrameBufferGuard& operator = (const RenderToFrameBufferGuard&) = delete;
public:
    explicit RenderToFrameBufferGuard(const FrameBuffer& fb);
    ~RenderToFrameBufferGuard();
private:
    const FrameBuffer* previous_frame_buffer_;
    static const FrameBuffer* last_frame_buffer_;
};

class RenderToScreenGuard {
    RenderToScreenGuard(const RenderToScreenGuard&) = delete;
    RenderToScreenGuard& operator = (const RenderToScreenGuard&) = delete;
public:
    RenderToScreenGuard();
    ~RenderToScreenGuard();
private:
    static bool is_active_;
};

}
