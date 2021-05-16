#include "RenderGuards.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <iostream>
#include <stdexcept>

using namespace Mlib;

const FrameBufferMsaa* RenderToFrameBufferGuard::last_frame_buffer_ = nullptr;
bool RenderToFrameBufferGuard::is_empty_ = true;

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
    is_empty_ = true;
}

RenderToFrameBufferGuard::~RenderToFrameBufferGuard() {
    last_frame_buffer_->unbind();
    last_frame_buffer_ = previous_frame_buffer_;
    if (is_empty_) {
        std::cerr << "WARNING: Frame buffer was not drawn" << std::endl;
    }
}

RenderToScreenGuard::RenderToScreenGuard() {
    if (RenderToFrameBufferGuard::last_frame_buffer_ != nullptr) {
        if (!RenderToFrameBufferGuard::is_empty_) {
            throw std::runtime_error("Frame buffer was already drawn");
        }
        RenderToFrameBufferGuard::last_frame_buffer_->bind();
        RenderToFrameBufferGuard::is_empty_ = false;
    }
}

RenderToScreenGuard::~RenderToScreenGuard()
{}
