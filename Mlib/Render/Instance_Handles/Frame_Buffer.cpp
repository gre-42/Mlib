#include "Frame_Buffer.hpp"
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Geometry/Material/Mipmap_Mode.hpp>
#include <Mlib/Geometry/Material/Texture_Target.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>
#include <Mlib/Render/Download/Download_As_Stb_Image.hpp>
#include <Mlib/Render/Instance_Handles/Texture.hpp>
#include <Mlib/Render/Instance_Handles/Wrap_Mode.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <stb_cpp/stb_image_load.hpp>

using namespace Mlib;

void FrameBufferConfig::print() const {
    linfo() << "FrameBufferConfig";
    linfo() << "  width: " << width;
    linfo() << "  height: " << height;
    linfo() << "  color_internal_format: " << color_internal_format;
    linfo() << "  color_format: " << color_format;
    linfo() << "  color_type: " << color_type;
    linfo() << "  color_magnifying_interpolation_mode: " <<
        interpolation_mode_to_string(color_magnifying_interpolation_mode);
    linfo() << "  depth_kind: " << (int)depth_kind;
    linfo() << "  with_mipmaps: " << (int)with_mipmaps;
    linfo() << "  nsamples_msaa: " << (int)nsamples_msaa;
}

FrameBufferStorage::FrameBufferStorage(SourceLocation loc)
    : create_loc_{ loc }
    , bind_loc_{ loc }
    , deallocation_token_{ render_deallocator.insert([this]() {deallocate(); }) }
{}

FrameBufferStorage::~FrameBufferStorage() {
    if (ContextQuery::is_initialized()) {
        deallocate();
    } else {
        gc_deallocate();
    }
}

void FrameBufferStorage::configure(const FrameBufferConfig& config)
{
    if ((status_ == FrameBufferStatus::UNINITIALIZED) || (config != config_))
    {
        deallocate();
        allocate(config);
    }
}

bool FrameBufferStorage::is_configured() const {
    return (frame_buffer_ != (GLuint)-1);
}

void FrameBufferStorage::allocate(const FrameBufferConfig& config)
{
    if (config.nsamples_msaa <= 0) {
        THROW_OR_ABORT("config.nsamples_msaa <= 0");
    }

    config_ = config;
    CHK(glGenFramebuffers(1, &frame_buffer_));
    CHK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_));

    // create a color attachment texture
    texture_color_ = std::make_shared<Texture>(
        generate_texture,
        TextureTarget::TEXTURE_2D,
        config_.color_format,
        config_.with_mipmaps,
        config_.color_magnifying_interpolation_mode,
        config_.wrap_s,
        config_.wrap_t,
        config_.border_color,
        1);     // layers
    if (config.nsamples_msaa == 1) {
        CHK(glBindTexture(GL_TEXTURE_2D, texture_color_->handle<GLuint>()));
        CHK(glTexImage2D(GL_TEXTURE_2D, 0, config.color_internal_format, config.width, config.height, 0, config.color_format, config.color_type, nullptr));
    } else {
#ifdef __ANDROID__
        THROW_OR_ABORT("MSAA not supported on android");
#else
        CHK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture_color_->handle<GLuint>()));
        CHK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, config.nsamples_msaa, (GLenum)config.color_internal_format, config.width, config.height, GL_TRUE));
#endif
    }
    if (config.color_magnifying_interpolation_mode == InterpolationMode::LINEAR) {
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    } else {
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    }
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, config.wrap_s));
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, config.wrap_t));
    if (config.nsamples_msaa == 1) {
        CHK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_color_->handle<GLuint>(), 0));
    } else {
#ifdef __ANDROID__
        THROW_OR_ABORT("MSAA not supported on android");
#else
        CHK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture_color_->handle<GLuint>(), 0));
#endif
    }

    if (config.depth_kind == FrameBufferChannelKind::TEXTURE) {
        // create a depth attachment texture
        texture_depth_ = std::make_shared<Texture>(
            generate_texture,
            TextureTarget::TEXTURE_2D,
            ColorMode::GRAYSCALE,
            MipmapMode::NO_MIPMAPS,
            InterpolationMode::NEAREST,
            FixedArray<WrapMode, 2>{
                wrap_mode_from_native(config_.wrap_s),
                wrap_mode_from_native(config_.wrap_t)},
            config_.border_color,
            1);     // layers
        if (config.nsamples_msaa == 1) {
            CHK(glBindTexture(GL_TEXTURE_2D, texture_depth_->handle<GLuint>()));
            CHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, config.width, config.height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr));
        } else {
#ifdef __ANDROID__
            THROW_OR_ABORT("MSAA not supported on android");
#else
            CHK(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture_depth_->handle<GLuint>()));
            CHK(glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, config.nsamples_msaa, GL_DEPTH_COMPONENT24, config.width, config.height, GL_TRUE));
#endif
        }
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        if (config.nsamples_msaa == 1) {
            CHK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture_depth_->handle<GLuint>(), 0));
        } else {
#ifdef __ANDROID__
            THROW_OR_ABORT("MSAA not supported on android");
#else
            CHK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, texture_depth_->handle<GLuint>(), 0));
#endif
        }
    } else if (config.depth_kind == FrameBufferChannelKind::ATTACHMENT) {
        // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
        CHK(glGenRenderbuffers(1, &depth_buffer_));
        CHK(glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        if (config.nsamples_msaa == 1) {
            CHK(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, config.width, config.height)); // use a single renderbuffer object for both a depth AND stencil buffer.
        } else {
            CHK(glRenderbufferStorageMultisample(GL_RENDERBUFFER, config.nsamples_msaa, GL_DEPTH24_STENCIL8, config.width, config.height));
        }
        CHK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_buffer_)); // now actually attach it
    } else if (config.depth_kind == FrameBufferChannelKind::NONE) {
        // Do nothing
    } else {
        THROW_OR_ABORT("Unknown frame buffer depth kind");
    }

    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        THROW_OR_ABORT("Framebuffer is not complete");
    }
    CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    // CHK(glClearColor(1.f, 1.f, 1.f, 1.f));
    // linfo() << "bind0 i 0";
    // CHK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer_));
    // linfo() << "bind0 i 1";
    // CHK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    // linfo() << "bind0 i 2";
    // // CHK(glEnable(GL_CULL_FACE));
    // linfo() << "bind0 i 3";
    // // CHK(glDisable(GL_CULL_FACE));
    // linfo() << "bind0 i 4";
    // // CHK(glDisable(GL_DEPTH_TEST));
    // linfo() << "bind0 i 5";
    // CHK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0));
    // linfo() << "bind0 i 6";
}

void FrameBufferStorage::deallocate() {
    if (frame_buffer_ != (GLuint)-1) {
        ABORT(glDeleteFramebuffers(1, &frame_buffer_));
        frame_buffer_ = (GLuint)-1;
    }
    texture_color_ = nullptr;
    texture_depth_ = nullptr;
    if (depth_buffer_ != (GLuint)-1) {
        ABORT(glDeleteRenderbuffers(1, &depth_buffer_));
        depth_buffer_ = (GLuint)-1;
    }
    status_ = FrameBufferStatus::UNINITIALIZED;
}

void FrameBufferStorage::gc_deallocate() {
    if (frame_buffer_ != (GLuint)-1) {
        render_gc_append_to_frame_buffers(frame_buffer_);
        frame_buffer_ = (GLuint)-1;
    }
    texture_color_ = nullptr;
    texture_depth_ = nullptr;
    if (depth_buffer_ != (GLuint)-1) {
        render_gc_append_to_render_buffers(depth_buffer_);
        depth_buffer_ = (GLuint)-1;
    }
    status_ = FrameBufferStatus::UNINITIALIZED;
}

void FrameBufferStorage::bind_draw(SourceLocation loc) const {
    if (status_ == FrameBufferStatus::BOUND) {
        lerr() << create_loc_.file_name() << ':' << create_loc_.line();
        lerr() << bind_loc_.file_name() << ':' << bind_loc_.line();
        THROW_OR_ABORT("Frame buffer has already been bound");
    }
    status_ = FrameBufferStatus::BOUND;
    bind_loc_ = loc;
    CHK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frame_buffer_));
}

void FrameBufferStorage::bind(SourceLocation loc) const {
    if (status_ == FrameBufferStatus::BOUND) {
        lerr() << create_loc_.file_name() << ':' << create_loc_.line();
        lerr() << bind_loc_.file_name() << ':' << bind_loc_.line();
        THROW_OR_ABORT("Frame buffer has already been bound");
    }
    status_ = FrameBufferStatus::BOUND;
    bind_loc_ = loc;
    CHK(glBindFramebuffer(config_.target, frame_buffer_));
}

void FrameBufferStorage::unbind() const {
    if (status_ != FrameBufferStatus::BOUND) {
        lerr() << create_loc_.file_name() << ':' << create_loc_.line();
        lerr() << bind_loc_.file_name() << ':' << bind_loc_.line();
        verbose_abort("Frame buffer has not been bound");
    }
    status_ = FrameBufferStatus::WRITTEN;
    CHK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

std::shared_ptr<ITextureHandle> FrameBufferStorage::texture_color() const {
    if (status_ != FrameBufferStatus::WRITTEN) {
        THROW_OR_ABORT("Frame buffer has not been written");
    }
    return texture_color_;
}

std::shared_ptr<ITextureHandle> FrameBufferStorage::texture_depth() const {
    if (status_ != FrameBufferStatus::WRITTEN) {
        THROW_OR_ABORT("Frame buffer has not been written");
    }
    return texture_depth_;
}

FrameBuffer::FrameBuffer(SourceLocation loc)
    : fb_{ loc }
    , ms_fb_{ loc }
{}

FrameBuffer::~FrameBuffer() = default;

void FrameBuffer::configure(const FrameBufferConfig& config) {
    config_ = config;
    auto config1 = config;
    config1.nsamples_msaa = 1;
    fb_.configure(config1);
    if (config_.nsamples_msaa != 1) {
        ms_fb_.configure({
            .width = config_.width,
            .height = config_.height,
            .color_internal_format = config.color_internal_format,
            .color_format = config.color_format,
            .depth_kind = config_.depth_kind,
            .wrap_s = config_.wrap_s,
            .wrap_t = config_.wrap_t,
            .nsamples_msaa = config_.nsamples_msaa});
    }
}

bool FrameBuffer::is_configured() const {
    return fb_.is_configured();
}

void FrameBuffer::bind(SourceLocation loc) {
    if (config_.nsamples_msaa == 1) {
        fb_.bind(loc);
    } else {
        ms_fb_.bind_draw(loc);
    }
}

void FrameBuffer::unbind(SourceLocation loc) {
    if (config_.nsamples_msaa == 1) {
        fb_.unbind();
    } else {
        ms_fb_.unbind();
        CHK(glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fb_.frame_buffer_));
        fb_.bind_draw(loc);
        CHK(glBlitFramebuffer(0, 0, config_.width, config_.height, 0, 0, config_.width, config_.height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST));
        fb_.unbind();
    }
    if (config_.with_mipmaps) {
        CHK(glBindTexture(GL_TEXTURE_2D, texture_color()->handle<GLuint>()));
        CHK(glGenerateMipmap(GL_TEXTURE_2D));
    }
}

std::shared_ptr<ITextureHandle> FrameBuffer::texture_color() const {
    return fb_.texture_color();
}

std::shared_ptr<ITextureHandle> FrameBuffer::texture_depth() const {
    return fb_.texture_depth();
}

StbInfo<uint8_t> FrameBuffer::color_to_stb_image(size_t nchannels) const {
    return download_as_stb_image(fb_.frame_buffer_, config_.width, config_.height, integral_cast<int>(nchannels), FlipMode::VERTICAL);
}

Array<float> FrameBuffer::color_to_array(size_t nchannels) const {
    return Mlib::color_to_array(fb_.frame_buffer_, config_.width, config_.height, integral_cast<int>(nchannels), FlipMode::VERTICAL);
}

Array<float> FrameBuffer::depth_to_array() const {
    return Mlib::depth_to_array(fb_.frame_buffer_, config_.width, config_.height, FlipMode::VERTICAL);
}

void FrameBuffer::deallocate() {
    fb_.deallocate();
    ms_fb_.deallocate();
}
