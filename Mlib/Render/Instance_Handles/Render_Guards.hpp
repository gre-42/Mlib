#pragma once
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <memory>

namespace Mlib {

class IFrameBuffer;

class RenderToFrameBufferGuard {
    RenderToFrameBufferGuard(const RenderToFrameBufferGuard&) = delete;
    RenderToFrameBufferGuard& operator = (const RenderToFrameBufferGuard&) = delete;
public:
    explicit RenderToFrameBufferGuard(std::shared_ptr<IFrameBuffer> fb);
    ~RenderToFrameBufferGuard();
private:
    std::shared_ptr<IFrameBuffer> previous_frame_buffer_;
};

void notify_rendering(SourceLocation loc);

}
