#include "Frame_Buffer_2D.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

FrameBufferStorage2D::FrameBufferStorage2D(GLuint texture_color, GLint level)
    : deallocation_token_{ render_deallocator.insert([this]() { deallocate(); }) }
{
    allocate(texture_color, level);
}

FrameBufferStorage2D::~FrameBufferStorage2D() {
    deallocate();
}

void FrameBufferStorage2D::allocate(GLuint texture_color, GLint level)
{
    CHK(glGenFramebuffers(1, &frame_buffer_));
    CHK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_));

    CHK(glBindTexture(GL_TEXTURE_2D, texture_color));

    CHK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_color, level));

    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        THROW_OR_ABORT("Framebuffer is not complete");
    }
    CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FrameBufferStorage2D::deallocate() {
    if (frame_buffer_ != (GLuint)-1) {
        ABORT(glDeleteFramebuffers(1, &frame_buffer_));
        frame_buffer_ = (GLuint)-1;
    }
}

bool FrameBufferStorage2D::is_configured() const {
    return true;
}

void FrameBufferStorage2D::bind(SourceLocation loc) {
    CHK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer_));
}

void FrameBufferStorage2D::unbind(SourceLocation loc) {
    CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}
