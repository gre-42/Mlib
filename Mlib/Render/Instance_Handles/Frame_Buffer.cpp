#include "Frame_Buffer.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Garbage_Collector.hpp>
#include <cassert>
#include <stdexcept>

using namespace Mlib;

FrameBuffer::FrameBuffer()
{}

FrameBuffer::~FrameBuffer() {
    if (glfwGetCurrentContext() != nullptr) {
        deallocate();
    } else {
        gc_deallocate();
    }
}

void FrameBuffer::configure(const FrameBufferConfig& config)
{
    if (config.width == -1 || config.height == -1) {
        throw std::runtime_error("Invalid width or height for FrameBuffer::begin_draw");
    }
    if (config != config_) {
        deallocate();
        allocate(config);
    }
}

void FrameBuffer::allocate(const FrameBufferConfig& config)
{
    if (config.nsamples_msaa  <= 0) {
        throw std::runtime_error("config.nsamples_msaa  <= 0");
    }

    config_ = config;
    CHK(glGenFramebuffers(1, &frame_buffer_));
    CHK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_));

    // create a color attachment texture
    CHK(glGenTextures(1, &texture_color_buffer));
    if (config.nsamples_msaa == 1) {
        CHK(glBindTexture(GL_TEXTURE_2D, texture_color_buffer));
        CHK(glTexImage2D(GL_TEXTURE_2D, 0, config.color_internal_format, config.width, config.height, 0, config.color_format, config.color_type, nullptr));
    } else {
        CHK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture_color_buffer));
        CHK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, config.nsamples_msaa, config.color_internal_format, config.width, config.height, GL_TRUE));
    }
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, config.color_filter_type));
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, config.color_filter_type));
    if (config.nsamples_msaa == 1) {
        CHK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_color_buffer, 0));
    } else {
        CHK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture_color_buffer, 0));
    }

    if (config.with_depth_texture) {
        // create a depth attachment texture
        CHK(glGenTextures(1, &texture_depth_buffer));
        if (config.nsamples_msaa == 1) {
            CHK(glBindTexture(GL_TEXTURE_2D, texture_depth_buffer));
            CHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, config.width, config.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr));
        } else {
            CHK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture_depth_buffer));
            CHK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, config.nsamples_msaa, GL_DEPTH_COMPONENT24, config.width, config.height, GL_TRUE));
        }
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        if (config.nsamples_msaa == 1) {
            CHK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture_depth_buffer, 0));
        } else {
            CHK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, texture_depth_buffer, 0));
        }
    } else {
        // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
        CHK(glGenRenderbuffers(1, &render_buffer));
        CHK(glBindRenderbuffer(GL_RENDERBUFFER, render_buffer));
        if (config.nsamples_msaa == 1) {
            CHK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, config.width, config.height)); // use a single renderbuffer object for both a depth AND stencil buffer.
        } else {
            CHK(glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, config.width, config.height));
        }
        CHK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_buffer)); // now actually attach it
    }

    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Framebuffer is not complete");
    }
    CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FrameBuffer::deallocate() {
    if (frame_buffer_ != (GLuint)-1) {
        WARN(glDeleteFramebuffers(1, &frame_buffer_));
        frame_buffer_ = (GLuint)-1;
    }
    if (texture_color_buffer != (GLuint)-1) {
        WARN(glDeleteTextures(1, &texture_color_buffer));
        texture_color_buffer = (GLuint)-1;
    }
    if (texture_depth_buffer != (GLuint)-1) {
        WARN(glDeleteTextures(1, &texture_depth_buffer));
        texture_depth_buffer = (GLuint)-1;
    }
    if (render_buffer != (GLuint)-1) {
        WARN(glDeleteRenderbuffers(1, &render_buffer));
        render_buffer = (GLuint)-1;
    }
}

void FrameBuffer::gc_deallocate() {
    if (frame_buffer_ != (GLuint)-1) {
        gc_frame_buffers.push_back(frame_buffer_);
        frame_buffer_ = (GLuint)-1;
    }
    if (texture_color_buffer != (GLuint)-1) {
        gc_texture_color_buffers.push_back(texture_color_buffer);
        texture_color_buffer = (GLuint)-1;
    }
    if (texture_depth_buffer != (GLuint)-1) {
        gc_texture_depth_buffers.push_back(texture_depth_buffer);
        texture_depth_buffer = (GLuint)-1;
    }
    if (render_buffer != (GLuint)-1) {
        gc_render_buffers.push_back(render_buffer);
        render_buffer = (GLuint)-1;
    }
}

void FrameBuffer::bind() const {
    CHK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_));
}

void FrameBuffer::unbind() const {
    CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FrameBufferMsaa::configure(const FrameBufferConfig& config) {
    config_ = config;
    auto config1 = config;
    config1.nsamples_msaa = 1;
    fb.configure(config1);
    if (config_.nsamples_msaa != 1) {
        ms_fb.configure({.width = config_.width, .height = config_.height, .with_depth_texture = config_.with_depth_texture, .nsamples_msaa = config_.nsamples_msaa});
    }
}

void FrameBufferMsaa::bind() const {
    if (config_.nsamples_msaa == 1) {
        fb.bind();
    } else {
        ms_fb.bind();
    }
}

void FrameBufferMsaa::unbind() const {
    if (config_.nsamples_msaa == 1) {
        fb.unbind();
    } else {
        ms_fb.unbind();
        CHK(glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fb.frame_buffer_));
        CHK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb.frame_buffer_));
        CHK(glBlitFramebuffer(0, 0, config_.width, config_.height, 0, 0, config_.width, config_.height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST));
        CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
}

void FrameBufferMsaa::deallocate() {
    fb.deallocate();
    ms_fb.deallocate();
}
