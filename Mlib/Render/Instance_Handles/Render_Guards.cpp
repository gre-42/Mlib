#include "Render_Guards.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <iostream>
#include <stdexcept>

using namespace Mlib;

const FrameBuffer* RenderToFrameBufferGuard::last_frame_buffer_ = nullptr;
bool RenderToScreenGuard::is_active_ = false;

// use cases:
// 1.
// {
//     RenderToScreenGuard rsg;
//     render();
// }
// 2.
// {
//     RenderToFrameBufferGuard rfg;
//     RenderToScreenGuard rsg;
//     render();
// }
// 3.
// {
//     RenderToFrameBufferGuard rfg0;
//     {
//         RenderToFrameBufferGuard rfg1;
//         RenderToScreenGuard rsg;
//         render();
//     }
//     {
//         RenderToScreenGuard rsg;
//         render();
//     }
//     glReadPixels();
// }

RenderToFrameBufferGuard::RenderToFrameBufferGuard(const FrameBuffer& fb)
: previous_frame_buffer_{last_frame_buffer_}
 {
    if (!fb.is_configured()) {
        throw std::runtime_error("Frame buffer has not been configured");
    }
    last_frame_buffer_ = &fb;
}

RenderToFrameBufferGuard::~RenderToFrameBufferGuard() {
    last_frame_buffer_->unbind();
    last_frame_buffer_ = previous_frame_buffer_;
}

RenderToScreenGuard::RenderToScreenGuard() {
    if (is_active_) {
        throw std::runtime_error("RenderToScreenGuard already active");
    }
    is_active_ = true;
    if (RenderToFrameBufferGuard::last_frame_buffer_ != nullptr) {
        RenderToFrameBufferGuard::last_frame_buffer_->bind();
    }
}

RenderToScreenGuard::~RenderToScreenGuard()
{
    is_active_ = false;
}
