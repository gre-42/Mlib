#include "RenderGuards.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <iostream>
#include <stdexcept>

using namespace Mlib;

const FrameBufferMsaa* RenderToFrameBufferGuard::first_frame_buffer_ = nullptr;
bool RenderToFrameBufferGuard::is_empty_ = true;
size_t RenderToFrameBufferGuard::stack_size_ = 0;

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

RenderToFrameBufferGuard::RenderToFrameBufferGuard(const FrameBufferMsaa& fb) {
    if (fb.fb.frame_buffer_ == (GLuint)-1) {
        throw std::runtime_error("Invalid input for RenderToFrameBufferGuard");
    }
    ++stack_size_;
    if (stack_size_ == 1) {
        first_frame_buffer_ = &fb;
        is_empty_ = true;
    } else {
        fb.bind();
    }
    last_frame_buffer_ = &fb;
}

RenderToFrameBufferGuard::~RenderToFrameBufferGuard() {
    if (stack_size_ == 1) {
        first_frame_buffer_ = nullptr;
    }
    last_frame_buffer_->unbind();
    --stack_size_;
}

RenderToScreenGuard::RenderToScreenGuard() {
    if (RenderToFrameBufferGuard::first_frame_buffer_ != nullptr) {
        if (!RenderToFrameBufferGuard::is_empty_) {
            throw std::runtime_error("Frame buffer was already drawn");
        }
        if (RenderToFrameBufferGuard::stack_size_ == 1) {
            RenderToFrameBufferGuard::first_frame_buffer_->bind();
            RenderToFrameBufferGuard::is_empty_ = false;
        }
    }
}

RenderToScreenGuard::~RenderToScreenGuard()
{}
