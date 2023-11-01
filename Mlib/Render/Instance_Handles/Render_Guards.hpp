#pragma once
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>

namespace Mlib {

class IFrameBuffer;

class RenderToFrameBufferGuard {
    friend class RenderToScreenGuard;
    RenderToFrameBufferGuard(const RenderToFrameBufferGuard&) = delete;
    RenderToFrameBufferGuard& operator = (const RenderToFrameBufferGuard&) = delete;
public:
    explicit RenderToFrameBufferGuard(const IFrameBuffer& fb);
    ~RenderToFrameBufferGuard();
private:
    const IFrameBuffer* previous_frame_buffer_;
    static const IFrameBuffer* last_frame_buffer_;
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
