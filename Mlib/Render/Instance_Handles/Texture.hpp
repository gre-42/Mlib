#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>

namespace Mlib {

static const struct GenerateTexture {} generate_texture;

class Texture: public ITextureHandle {
    Texture(const Texture&) = delete;
    Texture& operator = (const Texture&) = delete;
public:
    Texture(
        GenerateTexture,
        TextureTarget target,
        ColorMode color_mode,
        MipmapMode mipmap_mode,
        InterpolationMode magnifying_interpolation_mode,
        FixedArray<WrapMode, 2> wrap_modes,
        const FixedArray<float, 4>& border_color,
        uint32_t layers);
    Texture(
        GLuint handle,
        TextureTarget target,
        ColorMode color_mode,
        MipmapMode mipmap_mode,
        InterpolationMode magnifying_interpolation_mode,
        FixedArray<WrapMode, 2> wrap_modes,
        const FixedArray<float, 4>& border_color,
        uint32_t layers);
    Texture(
        GenerateTexture,
        TextureTarget target,
        GLenum format,
        bool with_mipmaps,
        InterpolationMode magnifying_interpolation_mode,
        GLint wrap_s,
        GLint wrap_t,
        const FixedArray<float, 4>& border_color,
        uint32_t layers);
    Texture(
        GLuint handle,
        TextureTarget target,
        GLenum format,
        bool with_mipmaps,
        InterpolationMode magnifying_interpolation_mode,
        GLint wrap_s,
        GLint wrap_t,
        const FixedArray<float, 4>& border_color,
        uint32_t layers);
    Texture(Texture&& other) noexcept;
    ~Texture();
    virtual uint32_t handle32() const override;
    virtual uint64_t handle64() const override;
    virtual uint32_t& handle32() override;
    virtual uint64_t& handle64() override;
    virtual bool texture_is_loaded_and_try_preload() override;
    virtual TextureTarget target() const override;
    virtual ColorMode color_mode() const override;
    virtual MipmapMode mipmap_mode() const override;
    virtual InterpolationMode magnifying_interpolation_mode() const override;
    virtual WrapMode wrap_modes(size_t i) const override;
    virtual FixedArray<float, 4> border_color() const override;
    virtual uint32_t layers() const override;
private:
    void deallocate();
    GLuint handle_;
    TextureTarget target_;
    ColorMode color_mode_;
    MipmapMode mipmap_mode_;
    InterpolationMode magnifying_interpolation_mode_;
    FixedArray<WrapMode, 2> wrap_modes_;
    FixedArray<float, 4> border_color_;
    uint32_t layers_;
    DeallocationToken deallocation_token_;
};

}
