#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer_Channel_Kind.hpp>
#include <Mlib/Render/Instance_Handles/IFrame_Buffer.hpp>
#include <compare>
#include <cstdint>

template <class TData>
class StbInfo;

namespace Mlib {

template <class TData>
class Array;
class ITextureHandle;
class FrameBuffer;
class RenderToFrameBufferGuard;

struct FrameBufferConfig {
    GLsizei width = -1;
    GLsizei height = -1;
    GLenum target = GL_DRAW_FRAMEBUFFER;
    GLint color_internal_format = GL_RGB;
    GLenum color_format = GL_RGB;
    GLenum color_type = GL_UNSIGNED_BYTE;
    GLint color_filter_type = GL_LINEAR;
    FrameBufferChannelKind depth_kind = FrameBufferChannelKind::ATTACHMENT;
    GLint wrap_s = GL_REPEAT;
    GLint wrap_t = GL_REPEAT;
    bool with_mipmaps = false;
    int nsamples_msaa = 1;
    auto operator <=> (const FrameBufferConfig&) const = default;
    void print() const;
};

enum FrameBufferStatus {
    UNINITIALIZED,
    BOUND,
    WRITTEN
};

class FrameBufferStorage {
    friend FrameBuffer;
    friend RenderToFrameBufferGuard;
    FrameBufferStorage(const FrameBufferStorage&) = delete;
    FrameBufferStorage& operator = (const FrameBufferStorage&) = delete;
public:
    explicit FrameBufferStorage(SourceLocation loc);
    ~FrameBufferStorage();
    void configure(const FrameBufferConfig& config);
    bool is_configured() const;
    void deallocate();
    void bind_draw(SourceLocation loc) const;
    void bind(SourceLocation loc) const;
    void unbind() const;
    std::shared_ptr<ITextureHandle> texture_color() const;
    std::shared_ptr<ITextureHandle> texture_depth() const;
private:
    void gc_deallocate();
    void allocate(const FrameBufferConfig& config);
    FrameBufferConfig config_;
    GLuint frame_buffer_ = (GLuint)-1;
    std::shared_ptr<ITextureHandle> texture_color_;
    std::shared_ptr<ITextureHandle> texture_depth_;
    GLuint depth_buffer_ = (GLuint)-1;
    mutable FrameBufferStatus status_ = FrameBufferStatus::UNINITIALIZED;
    mutable SourceLocation create_loc_;
    mutable SourceLocation bind_loc_;
    DeallocationToken deallocation_token_;
};

class FrameBuffer: public IFrameBuffer {
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator = (const FrameBuffer&) = delete;
public:
    explicit FrameBuffer(SourceLocation loc);
    virtual ~FrameBuffer() override;
    void configure(const FrameBufferConfig& config);
    virtual bool is_configured() const override;
    virtual void bind(SourceLocation loc) override;
    virtual void unbind(SourceLocation loc) override;
    void deallocate();
    std::shared_ptr<ITextureHandle> texture_color() const;
    std::shared_ptr<ITextureHandle> texture_depth() const;
    StbInfo<uint8_t> color_to_stb_image(size_t nchannels) const;
    Array<float> color_to_array(size_t nchannels) const;
    Array<float> depth_to_array() const;
private:
    FrameBufferStorage fb_;
    FrameBufferStorage ms_fb_;
    FrameBufferConfig config_;
};

}
