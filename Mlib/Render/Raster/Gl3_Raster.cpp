#include "Gl3_Raster.hpp"
#include <Mlib/Geometry/Material/Interpolation_Mode.hpp>
#include <Mlib/Geometry/Material/Texture_Target.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Dff.hpp>
#include <Mlib/Geometry/Mesh/Load/Mipmap_Level.hpp>
#include <Mlib/Geometry/Mesh/Load/Raster_Config.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Bind_Texture_Guard.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer_2D.hpp>
#include <Mlib/Render/Instance_Handles/Texture.hpp>
#include <Mlib/Render/Raster/Convert_Pixels.hpp>
#include <Mlib/Render/Raster/Gl3_Caps.hpp>

#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT                   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT                  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT                  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT                  0x83F3

using namespace Mlib::Dff;

void Gl3Raster::from_image(const Image& iimage) {
    void (*conv)(uint8_t *out, const uint8_t *in) = nullptr;

    // Unpalettize image if necessary but don't change original
    Image truecolimg;
    const Image* pimage;
    if (iimage.depth <= 8) {
        truecolimg.width = iimage.width;
        truecolimg.height = iimage.height;
        truecolimg.depth = iimage.depth;
        truecolimg.pixels = iimage.pixels;
        truecolimg.stride = iimage.stride;
        truecolimg.palette = iimage.palette;
        truecolimg.bpp = iimage.bpp;
        truecolimg.unpalletize();
        pimage = &truecolimg;
    } else {
        pimage = &iimage;
    }

    uint32_t fmt = format_ & 0xF00;
    if (compression_ != 0) {
        THROW_OR_ABORT("Texture is compressed");
    }
    switch (pimage->depth) {
    case 32:
        if (gl3Caps.gles)
            conv = conv_RGBA8888_from_RGBA8888;
        else if (fmt == Raster::C8888)
            conv = conv_RGBA8888_from_RGBA8888;
        else if(fmt == Raster::C888)
            conv = conv_RGB888_from_RGB888;
        else
            goto err;
        break;
    case 24:
        if (gl3Caps.gles)
            conv = conv_RGBA8888_from_RGB888;
        else if(fmt == Raster::C8888)
            conv = conv_RGBA8888_from_RGB888;
        else if(fmt == Raster::C888)
            conv = conv_RGB888_from_RGB888;
        else
            goto err;
        break;
    case 16:
        if (gl3Caps.gles)
            conv = conv_RGBA8888_from_ARGB1555;
        else if(fmt == Raster::C1555)
            conv = conv_RGBA5551_from_ARGB1555;
        else
            goto err;
        break;

    case 8:
    case 4:
    default:
    err:
        THROW_OR_ABORT("Unsupported image depth");
    }

    if (has_alpha_ != pimage->has_alpha()) {
        THROW_OR_ABORT("Conflicting alpha attribute");
    }

    bool unlock_required = false;
    if (levels_.empty()) {
        THROW_OR_ABORT("Number of mipmaps is 0");
    }
    if (pixels_.empty()) {
        lock(0, Raster::LOCKWRITE | Raster::LOCKNOFETCH);
        unlock_required = true;
    }

    if (pixels_.size() != stride_ * height_) {
        THROW_OR_ABORT("Unexpected number of allocated pixels");
    }
    const uint8_t *imgpixels = pimage->pixels.data() + (pimage->height - 1) * pimage->stride;

    if (pimage->width != width_) {
        THROW_OR_ABORT("Unexpected image width");
    }
    if (pimage->height != height_) {
        THROW_OR_ABORT("Unexpected image height");
    }
    uint8_t* ppixels = flip_y_axis_
        ? pixels_.data() + stride_ * (pimage->height - 1)
        : pixels_.data();
    for (uint32_t y = 0; y < pimage->height; y++) {
        const uint8_t *imgrow = imgpixels;
        uint8_t *rasrow = ppixels;
        for(uint32_t x = 0; x < pimage->width; x++){
            conv(rasrow, imgrow);
            imgrow += pimage->bpp;
            rasrow += native_bpp_;
        }
        imgpixels -= pimage->stride;
        if (flip_y_axis_) {
            ppixels -= stride_;
        } else {
            ppixels += stride_;
        }
    }
    if (unlock_required)
        unlock();
}

Image Gl3Raster::to_image() {
    THROW_OR_ABORT("Gl3Raster::to_image not implemented");
}

uint32_t Gl3Raster::width() const {
    return width_;
}

uint32_t Gl3Raster::height() const {
    return height_;
}

const MipmapLevel& Gl3Raster::mipmap_level(uint32_t level) const
{
    if (level >= levels_.size()) {
        THROW_OR_ABORT("Mipmap level out of bounds");
    }
    return levels_[level];
}

uint32_t Gl3Raster::num_levels() const {
    return integral_cast<uint32_t>(levels_.size());
}

void Gl3Raster::allocate_dxt(const RasterConfig& cfg) {
    if (type() != Raster::TEXTURE) {
        THROW_OR_ABORT("allocate_dxt requires \"type == texture\"");
    }
    switch (compression_) {
    case 1:
        if (has_alpha_) {
            native_internal_format_ = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            native_format_ = GL_RGBA;
        }
        else {
            native_internal_format_ = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
            native_format_ = GL_RGB;
        }
        // bogus, but stride*height should be the size of the image
        // 4x4 in 8 bytes
        stride_ = width_ / 2;
        break;
    case 3:
        native_internal_format_ = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        native_format_ = GL_RGBA;
        // 4x4 in 16 bytes
        stride_ = width_;
        break;
    case 5:
        native_internal_format_ = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        native_format_ = GL_RGBA;
        // 4x4 in 16 bytes
        stride_ = width_;
        break;
    default:
        THROW_OR_ABORT("Invalid DXT format");
    }
    native_type_ = GL_UNSIGNED_BYTE;
    native_has_alpha_ = has_alpha_;
    native_bpp_ = 2;
    depth_ = 16;

    native_is_compressed_ = true;
    if (format_ & Raster::MIPMAP)
        native_num_levels_ = num_levels_;
    native_autogen_mipmap_ = (format_ & (Raster::MIPMAP | Raster::AUTOMIPMAP)) == (Raster::MIPMAP | Raster::AUTOMIPMAP);
    if (native_autogen_mipmap_)
        native_num_levels_ = 1;

    if (native_texture_id_ != nullptr) {
        THROW_OR_ABORT("Texture already set");
    }
    native_texture_id_ = std::make_shared<Mlib::Texture>(
        generate_texture,
        TextureTarget::TEXTURE_2D,
        native_format_,
        bool(format_ & Raster::MIPMAP),
        InterpolationMode::LINEAR,
        GL_REPEAT,
        GL_REPEAT,
        FixedArray<float, 4>{1.f, 0.f, 1.f, 1.f},
        1);     // layers
    {
        BindTextureGuard btg{ GL_TEXTURE_2D, native_texture_id_->handle<GLuint>() };
        CHK(glTexImage2D(GL_TEXTURE_2D, 0, integral_cast<GLint>(native_internal_format_),
            integral_cast<GLsizei>(width_), integral_cast<GLsizei>(height_),
            0, native_format_, native_type_, nullptr));
        // TODO: allocate other levels...probably
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, integral_cast<GLint>(native_num_levels_ - 1)));
        // native_filterMode = 0;
        // native_addressU = 0;
        // native_addressV = 0;
        // native_maxAnisotropy = 1;
    }

    format_ &= ~Raster::DONTALLOCATE;
}

uint8_t* Gl3Raster::lock(uint32_t level, uint32_t lock_mode)
{
    if (private_flags_ != 0) {
        THROW_OR_ABORT("Raster private flags not zero");
    }

    switch (type()) {
    case Raster::NORMAL:
    case Raster::TEXTURE:
    case Raster::CAMERATEXTURE: {
        const auto& level_meta_data = mipmap_level(level);
        auto alloc_sz = level_meta_data.size();
        pixels_.resize(alloc_sz);

        if (lock_mode & Raster::LOCKREAD || !(lock_mode & Raster::LOCKNOFETCH)) {
            if (native_is_compressed_) {
                if (!levels_.empty()) {
                    if (level > levels_.size()) {
                        THROW_OR_ABORT("Mipmap level too large");
                    }
                    if (alloc_sz != levels_[level].data.size()) {
                        THROW_OR_ABORT("Unexpected level data size");
                    }
                    std::memcpy(pixels_.data(), levels_[level].data.data(), alloc_sz);
                } else {
                    // GLES is losing here
#ifdef __ANDROID__
                    THROW_OR_ABORT("glGetCompressedTexImage not supported on Android. Alternative: https://stackoverflow.com/a/53993894/2292832");
#else
                    BindTextureGuard btg{ GL_TEXTURE_2D, native_texture_id_->handle<GLuint>() };
                    CHK(glGetCompressedTexImage(GL_TEXTURE_2D, integral_cast<GLint>(level), pixels_.data()));
#endif
                }
            } else if (gl3Caps.gles) {
                if (native_format_ != GL_RGBA) {
                    THROW_OR_ABORT("Unexpected native format");
                }
                FrameBufferStorage2D fbs{ native_texture_id_->handle<GLuint>(), 0 };
                CHK(glReadPixels(0, 0, integral_cast<GLsizei>(level_meta_data.width), integral_cast<GLsizei>(level_meta_data.height), native_format_, native_type_, pixels_.data()));
                //e = glGetError(); printf("GL err4 %x (%x)\n", e, native_format);
            } else {
#ifdef __ANDROID__
                THROW_OR_ABORT("glGetTexImage not supported on Android. Alternative: https://stackoverflow.com/a/53993894/2292832");
#else
                BindTextureGuard btg{ GL_TEXTURE_2D, native_texture_id_->handle<GLuint>() };
                CHK(glPixelStorei(GL_PACK_ALIGNMENT, 1));
                CHK(glGetTexImage(GL_TEXTURE_2D, integral_cast<GLint>(level), native_format_, native_type_, pixels_.data()));
#endif
            }
        }

        private_flags_ = lock_mode;
        break;
    }

    case Raster::CAMERA: {
        if (lock_mode & Raster::PRIVATELOCK_WRITE) {
            THROW_OR_ABORT("Can't lock framebuffer for writing");
        }
        if (native_bpp_ != 3) {
            THROW_OR_ABORT("Camera raster is not RGB");
        }
        auto alloc_sz = height_ * stride_;
        pixels_.resize(alloc_sz);
        CHK(glReadBuffer(GL_BACK));
        CHK(glReadPixels(0, 0, integral_cast<GLint>(width_), integral_cast<GLint>(height_), GL_RGB, GL_UNSIGNED_BYTE, pixels_.data()));

        private_flags_ = lock_mode;
        break;
    }

    default:
        THROW_OR_ABORT("Cannot lock this type of raster yet");
    }
    locked_level_ = level;
    return pixels_.data();
}

void Gl3Raster::unlock()
{
    if (pixels_.empty()) {
        THROW_OR_ABORT("No raster pixels allocated");
    }
    if (!locked_level_.has_value()) {
        THROW_OR_ABORT("Raster unlock without previous lock");
    }
    auto level = *locked_level_;

    switch (type()) {
    case Raster::NORMAL:
    case Raster::TEXTURE:
    case Raster::CAMERATEXTURE:
        if (private_flags_ & Raster::LOCKWRITE) {
            const auto& level_meta_data = mipmap_level(level);
            BindTextureGuard btg{ GL_TEXTURE_2D, native_texture_id_->handle<GLuint>() };
            if (native_is_compressed_) {
                CHK(glCompressedTexImage2D(
                    GL_TEXTURE_2D,
                    integral_cast<GLint>(level),
                    integral_cast<GLenum>(native_internal_format_),
                    integral_cast<GLsizei>(level_meta_data.width),
                    integral_cast<GLsizei>(level_meta_data.height),
                    0,
                    integral_cast<GLsizei>(level_meta_data.size()),
                    pixels_.data()));
                if (!levels_.empty()) {
                    if (level >= levels_.size()) {
                        THROW_OR_ABORT("Level index too large for backing-store");
                    }
                    memcpy(levels_[level].data.data(), pixels_.data(),
                        levels_[level].data.size());
                }
            } else {
                CHK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
                CHK(glTexImage2D(
                    GL_TEXTURE_2D,
                    integral_cast<GLint>(level),
                    integral_cast<GLint>(native_internal_format_),
                    integral_cast<GLsizei>(level_meta_data.width),
                    integral_cast<GLsizei>(level_meta_data.height),
                    0,
                    native_format_,
                    native_type_,
                    pixels_.data()));
            }
            if (level == 0 && native_autogen_mipmap_)
                CHK(glGenerateMipmap(GL_TEXTURE_2D));
        }
        break;

    case Raster::CAMERA:
        // TODO: write?
        break;
    }

    pixels_.clear();
    private_flags_ = 0;
    locked_level_.reset();
}

void Gl3Raster::create_texture()
{
    if (format_ & (Raster::PAL4 | Raster::PAL8)) {
        THROW_OR_ABORT("Raster is not a texture");
    }

    switch(format_ & 0xF00){
    case Raster::C8888:
        native_internal_format_ = GL_RGBA8;
        native_format_ = GL_RGBA;
        native_type_ = GL_UNSIGNED_BYTE;
        native_has_alpha_ = 1;
        native_bpp_ = 4;
        depth_ = 32;
        break;
    case Raster::C888:
        native_internal_format_ = GL_RGB8;
        native_format_ = GL_RGB;
        native_type_ = GL_UNSIGNED_BYTE;
        native_has_alpha_ = 0;
        native_bpp_ = 3;
        depth_ = 24;
        break;
    case Raster::C1555:
        native_internal_format_ = GL_RGB5_A1;
        native_format_ = GL_RGBA;
        native_type_ = GL_UNSIGNED_SHORT_5_5_5_1;
        native_has_alpha_ = 1;
        native_bpp_ = 2;
        depth_ = 16;
        break;
    default:
        THROW_OR_ABORT("Invalid raster");
    }

    if (gl3Caps.gles) {
        // glReadPixels only supports GL_RGBA
        native_internal_format_ = GL_RGBA8;
        native_format_ = GL_RGBA;
        native_type_ = GL_UNSIGNED_BYTE;
        native_bpp_ = 4;
    }

    stride_ = width_ * native_bpp_;

    if (format_ & Raster::MIPMAP) {
        uint32_t w = width_;
        uint32_t h = height_;
        native_num_levels_ = 1;
        while (w != 1 || h != 1) {
            native_num_levels_++;
            if(w > 1) w /= 2;
            if(h > 1) h /= 2;
        }
    }
    native_autogen_mipmap_ = (format_ & (Raster::MIPMAP|Raster::AUTOMIPMAP)) == (Raster::MIPMAP|Raster::AUTOMIPMAP);
    if (native_autogen_mipmap_)
        native_num_levels_ = 1;

    if (native_texture_id_ != nullptr) {
        THROW_OR_ABORT("Texture ID already set");
    }
    native_texture_id_ = std::make_shared<Mlib::Texture>(
        generate_texture,
        TextureTarget::TEXTURE_2D,
        native_format_,
        bool(format_ & Raster::MIPMAP),
        InterpolationMode::LINEAR,
        GL_REPEAT,
        GL_REPEAT,
        FixedArray<float, 4>{1.f, 0.f, 1.f, 1.f},
        1);     // layers
    BindTextureGuard btg{ GL_TEXTURE_2D, native_texture_id_->handle<GLuint>() };
    CHK(glTexImage2D(
        GL_TEXTURE_2D,
        0,
        integral_cast<GLint>(native_internal_format_),
        integral_cast<GLsizei>(width_),
        integral_cast<GLsizei>(height_),
        0,
        native_format_,
        native_type_,
        nullptr));
    // TODO: allocate other levels...probably
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, integral_cast<GLint>(native_num_levels_ - 1)));
}

Gl3Raster::Gl3Raster(
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    uint32_t format,
    uint32_t compression,
    uint32_t num_levels,
    bool has_alpha,
    const RasterConfig& cfg)
    : num_levels_{ num_levels }
    , format_{ format }
    , compression_{ compression }
    , has_alpha_{ has_alpha }
    , width_{ width }
    , height_{ height }
    , depth_{ depth }
    , private_flags_{ 0 }
    , flip_y_axis_{ cfg.flip_gl_y_axis }
{
    if ((format & 0xF) != Raster::TEXTURE) {
        THROW_OR_ABORT("Invalid raster type");
    }

    native_is_compressed_ = false;
    native_has_alpha_ = false;
    native_num_levels_ = 1;

    if (width_ == 0 || height_ == 0) {
        format_ |= Raster::DONTALLOCATE;
        stride_ = 0;
        return;
    }
    if(format_ & Raster::DONTALLOCATE)
        return;

    switch(type()){
    case Raster::NORMAL:
    case Raster::TEXTURE:
        create_texture();
        break;
    case Raster::CAMERATEXTURE:
        THROW_OR_ABORT("Camera texture not yet supported");
        break;
    case Raster::ZBUFFER:
        THROW_OR_ABORT("z-buffer texture not yet supported");
        break;
    case Raster::CAMERA:
        THROW_OR_ABORT("Camera not yet supported");
        break;
    default:
        THROW_OR_ABORT("Unknown raster type");
    }

    if (compression != 0) {
        verbose_abort("This code is not yet fully implemented, it produces a resource leak by discarding the previously allocated texture");
        allocate_dxt(cfg);
    }
    levels_ = MipmapLevel::compute(
        width_,
        height_,
        stride_,
        native_internal_format_,
        native_num_levels_,
        // Uh oh, need to keep a copy in cpu memory
        (gl3Caps.gles && cfg.need_to_read_back_textures)
        ? AllocationMode::ALLOCATE
        : AllocationMode::NO_ALLOCATE);
}

Gl3Raster::~Gl3Raster() = default;

uint32_t Gl3Raster::type() const {
    return format_ & 0x7;
}

uint32_t Gl3Raster::flags() const {
    return format_ & 0xF8;
}

uint32_t Gl3Raster::format() const {
    return format_ & 0xFF00;
}

std::shared_ptr<Mlib::ITextureHandle> Gl3Raster::texture_handle() {
    if (native_texture_id_ == nullptr) {
        THROW_OR_ABORT("Texture ID not set");
    }
    return native_texture_id_;
}
