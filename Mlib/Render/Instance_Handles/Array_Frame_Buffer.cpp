#include "Array_Frame_Buffer.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ArrayFrameBufferStorage::ArrayFrameBufferStorage(GLuint texture_color, GLint level, GLint layer)
    : deallocation_token_{ render_deallocator.insert([this]() {deallocate(); }) }
{
    allocate(texture_color, level, layer);
}

ArrayFrameBufferStorage::~ArrayFrameBufferStorage() {
    deallocate();
}

void ArrayFrameBufferStorage::allocate(GLuint texture_color, GLint level, GLint layer)
{
    CHK(glGenFramebuffers(1, &frame_buffer_));
    CHK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_));

    CHK(glBindTexture(GL_TEXTURE_2D_ARRAY, texture_color));

    CHK(glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_color, level, layer));

    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        THROW_OR_ABORT("Framebuffer is not complete");
    }
    CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void ArrayFrameBufferStorage::deallocate() {
    if (frame_buffer_ != (GLuint)-1) {
        ABORT(glDeleteFramebuffers(1, &frame_buffer_));
        frame_buffer_ = (GLuint)-1;
    }
}

bool ArrayFrameBufferStorage::is_configured() const {
    return true;
}

void ArrayFrameBufferStorage::bind(SourceLocation loc) {
    CHK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer_));
}

void ArrayFrameBufferStorage::unbind(SourceLocation loc) {
    CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}
