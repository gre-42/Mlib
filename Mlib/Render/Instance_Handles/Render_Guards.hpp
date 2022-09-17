#pragma once
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>

namespace Mlib {

class FrameBuffer;

class RenderToFrameBufferGuard {
    friend class RenderToScreenGuard;
public:
    explicit RenderToFrameBufferGuard(const FrameBuffer& fb);
    ~RenderToFrameBufferGuard();
private:
    const FrameBuffer* previous_frame_buffer_;
    static const FrameBuffer* last_frame_buffer_;
};

class RenderToScreenGuard {
public:
    RenderToScreenGuard();
    ~RenderToScreenGuard();
private:
    static bool is_active_;
};

}
