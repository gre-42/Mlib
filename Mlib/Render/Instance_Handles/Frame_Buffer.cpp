#include "Frame_Buffer.hpp"
#include <Mlib/Render/CHK.hpp>
#include <stdexcept>

using namespace Mlib;

FrameBuffer::FrameBuffer()
{}

FrameBuffer::~FrameBuffer() {
    if (glfwGetCurrentContext() != nullptr) {
        deallocate();
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
    config_ = config;
    CHK(glGenFramebuffers(1, &frame_buffer));
    CHK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer));

    // create a color attachment texture
    CHK(glGenTextures(1, &texture_color_buffer));
    CHK(glBindTexture(GL_TEXTURE_2D, texture_color_buffer));
    CHK(glTexImage2D(GL_TEXTURE_2D, 0, config.color_internal_format, config.width, config.height, 0, config.color_format, config.color_type, nullptr));
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, config.color_filter_type));
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, config.color_filter_type));
    CHK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_color_buffer, 0));

    if (config.with_depth_texture) {
        // create a depth attachment texture
        CHK(glGenTextures(1, &texture_depth_buffer));
        CHK(glBindTexture(GL_TEXTURE_2D, texture_depth_buffer));
        CHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, config.width, config.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        CHK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture_depth_buffer, 0));
    } else {
        // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
        CHK(glGenRenderbuffers(1, &render_buffer));
        CHK(glBindRenderbuffer(GL_RENDERBUFFER, render_buffer));
        CHK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, config.width, config.height)); // use a single renderbuffer object for both a depth AND stencil buffer.
        CHK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_buffer)); // now actually attach it
    }

    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Framebuffer is not complete");
    }
    CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FrameBuffer::deallocate() {
    if (frame_buffer != (GLuint)-1) {
        WARN(glDeleteFramebuffers(1, &frame_buffer));
        frame_buffer = (GLuint)-1;
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
