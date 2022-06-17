#include "Render_Guards.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <iostream>
#include <stdexcept>

using namespace Mlib;

const FrameBufferMsaa* RenderToFrameBufferGuard::last_frame_buffer_ = nullptr;

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

RenderToFrameBufferGuard::RenderToFrameBufferGuard(const FrameBufferMsaa& fb)
: previous_frame_buffer_{last_frame_buffer_}
 {
    if (fb.fb.frame_buffer_ == (GLuint)-1) {
        throw std::runtime_error("Invalid input for RenderToFrameBufferGuard");
    }
    last_frame_buffer_ = &fb;
}

RenderToFrameBufferGuard::~RenderToFrameBufferGuard() {
    last_frame_buffer_->unbind();
    last_frame_buffer_ = previous_frame_buffer_;
}

RenderToScreenGuard::RenderToScreenGuard() {
    if (RenderToFrameBufferGuard::last_frame_buffer_ != nullptr) {
        RenderToFrameBufferGuard::last_frame_buffer_->bind();
    }
}

RenderToScreenGuard::~RenderToScreenGuard()
{}
