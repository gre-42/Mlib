#include "RenderGuards.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <iostream>
#include <stdexcept>

using namespace Mlib;

GLuint RenderToFrameBufferGuard::first_frame_buffer_ = (GLuint)-1;
bool RenderToFrameBufferGuard::is_empty_ = true;
size_t RenderToFrameBufferGuard::stack_size_ = 0;

RenderToFrameBufferGuard::RenderToFrameBufferGuard(const FrameBuffer& fb) {
    if (fb.frame_buffer_ == (GLuint)-1) {
        throw std::runtime_error("Invalid input for RenderToFrameBufferGuard");
    }
    ++stack_size_;
    if (stack_size_ == 1) {
        first_frame_buffer_ = fb.frame_buffer_;
        is_empty_ = true;
    } else {
        last_frame_buffer_ = fb.frame_buffer_;
        CHK(glBindFramebuffer(GL_FRAMEBUFFER, last_frame_buffer_));
    }
}

RenderToFrameBufferGuard::~RenderToFrameBufferGuard() {
    if (stack_size_ == 1) {
        first_frame_buffer_ = -1;
    } else {
        CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
    --stack_size_;
}

RenderToScreenGuard::RenderToScreenGuard() {
    if (RenderToFrameBufferGuard::first_frame_buffer_ != (GLuint)-1) {
        if (!RenderToFrameBufferGuard::is_empty_) {
            throw std::runtime_error("Frame buffer was already drawn");
        }
        if (RenderToFrameBufferGuard::stack_size_ == 1) {
            CHK(glBindFramebuffer(GL_FRAMEBUFFER, RenderToFrameBufferGuard::first_frame_buffer_));
            RenderToFrameBufferGuard::is_empty_ = false;
        }
    }
}

RenderToScreenGuard::~RenderToScreenGuard() {
    if (RenderToFrameBufferGuard::first_frame_buffer_ != (GLuint)-1) {
        if (RenderToFrameBufferGuard::stack_size_ == 1) {
            CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        }
    }
}
