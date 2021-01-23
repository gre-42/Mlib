#include "RenderGuards.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <iostream>
#include <stdexcept>

using namespace Mlib;

const FrameBufferMsaa* RenderToFrameBufferGuard::first_frame_buffer_ = nullptr;
bool RenderToFrameBufferGuard::is_empty_ = true;
size_t RenderToFrameBufferGuard::stack_size_ = 0;

RenderToFrameBufferGuard::RenderToFrameBufferGuard(const FrameBufferMsaa& fb) {
    if (fb.fb.frame_buffer_ == (GLuint)-1) {
        throw std::runtime_error("Invalid input for RenderToFrameBufferGuard");
    }
    ++stack_size_;
    if (stack_size_ == 1) {
        first_frame_buffer_ = &fb;
        is_empty_ = true;
    } else {
        last_frame_buffer_ = &fb;
        fb.bind();
    }
}

RenderToFrameBufferGuard::~RenderToFrameBufferGuard() {
    if (stack_size_ == 1) {
        first_frame_buffer_ = nullptr;
    } else {
        last_frame_buffer_->unbind();
    }
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

RenderToScreenGuard::~RenderToScreenGuard() {
    if (RenderToFrameBufferGuard::first_frame_buffer_ != nullptr) {
        if (RenderToFrameBufferGuard::stack_size_ == 1) {
            RenderToFrameBufferGuard::first_frame_buffer_->unbind();
        }
    }
}
