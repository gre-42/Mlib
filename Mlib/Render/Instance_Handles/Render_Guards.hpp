#pragma once
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <memory>

namespace Mlib {

class IFrameBuffer;

class RenderToFrameBufferGuard {
    friend class RenderToScreenGuard;
    RenderToFrameBufferGuard(const RenderToFrameBufferGuard&) = delete;
    RenderToFrameBufferGuard& operator = (const RenderToFrameBufferGuard&) = delete;
public:
    explicit RenderToFrameBufferGuard(std::shared_ptr<IFrameBuffer> fb);
    ~RenderToFrameBufferGuard();
private:
    std::shared_ptr<IFrameBuffer> previous_frame_buffer_;
};

class RenderToScreenGuard {
    RenderToScreenGuard(const RenderToScreenGuard&) = delete;
    RenderToScreenGuard& operator = (const RenderToScreenGuard&) = delete;
public:
    RenderToScreenGuard(SourceLocation loc);
    ~RenderToScreenGuard();
private:
    bool is_active_;
};

}
