#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Mlib {

struct FrameBufferConfig {
    friend struct FrameBuffer;
    GLsizei width = -1;
    GLsizei height = -1;
    GLint color_internal_format = GL_RGB;
    GLenum color_format = GL_RGB;
    GLenum color_type = GL_UNSIGNED_BYTE;
    GLint color_filter_type = GL_LINEAR;
    bool with_depth_texture = false;
    int nsamples_msaa = 1;
    auto operator <=> (const FrameBufferConfig&) const = default;
};

struct FrameBuffer {
    FrameBuffer();
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator = (const FrameBuffer&) = delete;
    ~FrameBuffer();
    GLuint frame_buffer_ = (GLuint)-1;
    GLuint texture_color_buffer = (GLuint)-1;
    GLuint texture_depth_buffer = (GLuint)-1;
    GLuint render_buffer = (GLuint)-1;
    void configure(const FrameBufferConfig& config = FrameBufferConfig{});
    void deallocate();
    void bind() const;
    void unbind() const;
private:
    void gc_deallocate();
    void allocate(const FrameBufferConfig& config);
    FrameBufferConfig config_;
};

struct FrameBufferMsaa {
    FrameBuffer fb;
    FrameBuffer ms_fb;
    void configure(const FrameBufferConfig& config = FrameBufferConfig{});
    void bind() const;
    void unbind() const;
    void deallocate();
private:
    FrameBufferConfig config_;
};

}
