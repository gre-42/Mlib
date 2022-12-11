#pragma once
#ifdef __ANDROID__
#include <GLES3/gl32.h>
#else
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#include <compare>

namespace Mlib {

class FrameBuffer;
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

class FrameBufferStorage {
    friend FrameBuffer;
    friend RenderToFrameBufferGuard;
public:
    FrameBufferStorage();
    FrameBufferStorage(const FrameBufferStorage&) = delete;
    FrameBufferStorage& operator = (const FrameBufferStorage&) = delete;
    ~FrameBufferStorage();
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

class FrameBuffer {
public:
    void configure(const FrameBufferConfig& config);
    bool is_configured() const;
    void bind() const;
    void unbind() const;
    void deallocate();
    GLuint texture_color() const;
    GLuint texture_depth() const;
private:
    FrameBufferStorage fb_;
    FrameBufferStorage ms_fb_;
    FrameBufferConfig config_;
};

}
