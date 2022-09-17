#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <compare>

namespace Mlib {

struct FrameBufferMsaa;
class RenderToFrameBufferGuard;

struct FrameBufferConfig {
    GLsizei width = -1;
    GLsizei height = -1;
    GLint color_internal_format = GL_RGB;
    GLenum color_format = GL_RGB;
    GLenum color_type = GL_UNSIGNED_BYTE;
    GLint color_filter_type = GL_LINEAR;
    bool with_depth_texture = false;
    bool with_mipmaps = false;
    int nsamples_msaa = 1;
    auto operator <=> (const FrameBufferConfig&) const = default;
};

enum FrameBufferStatus {
    UNINITIALIZED,
    BOUND,
    WRITTEN
};

struct FrameBuffer {
    friend FrameBufferMsaa;
    friend RenderToFrameBufferGuard;
    FrameBuffer();
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator = (const FrameBuffer&) = delete;
    ~FrameBuffer();
    void configure(const FrameBufferConfig& config);
    bool is_configured() const;
    void deallocate();
    void bind() const;
    void bind_draw() const;
    void unbind() const;
    GLuint texture_color() const;
    GLuint texture_depth() const;
private:
    void gc_deallocate();
    void allocate(const FrameBufferConfig& config);
    FrameBufferConfig config_;
    GLuint frame_buffer_ = (GLuint)-1;
    GLuint texture_color_ = (GLuint)-1;
    GLuint texture_depth_ = (GLuint)-1;
    GLuint render_buffer_ = (GLuint)-1;
    mutable FrameBufferStatus status_ = FrameBufferStatus::UNINITIALIZED;
};

struct FrameBufferMsaa {
    void configure(const FrameBufferConfig& config);
    bool is_configured() const;
    void bind() const;
    void unbind() const;
    void deallocate();
    GLuint texture_color() const;
    GLuint texture_depth() const;
private:
    FrameBuffer fb_;
    FrameBuffer ms_fb_;
    FrameBufferConfig config_;
};

}
