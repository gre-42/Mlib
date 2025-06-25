#include "Texture.hpp"
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Geometry/Material/Mipmap_Mode.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Try_Delete.hpp>
#include <Mlib/Render/Instance_Handles/Wrap_Mode.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static ColorMode format_to_color_mode(GLenum format) {
    switch (format) {
    case GL_RED:
        return ColorMode::GRAYSCALE;
    case GL_RGB:
        return ColorMode::RGB;
    case GL_RGBA:
        return ColorMode::RGBA;
    default:
        THROW_OR_ABORT("Unsupported color mode: " + std::to_string(format));
    };
}

static inline GLuint check_handle(GLuint handle) {
    if (handle == (GLuint)-1) {
        THROW_OR_ABORT("Unsupported texture handle");
    }
    return handle;
}

Texture::Texture(
    GenerateTexture,
    TextureTarget target,
    ColorMode color_mode,
    MipmapMode mipmap_mode,
    InterpolationMode magnifying_interpolation_mode,
    FixedArray<WrapMode, 2> wrap_modes,
    const FixedArray<float, 4>& border_color,
    uint32_t layers)
    : handle_{ (GLuint)-1 }
    , target_{ target }
    , color_mode_{ color_mode }
    , mipmap_mode_{ mipmap_mode }
    , magnifying_interpolation_mode_{ magnifying_interpolation_mode }
    , wrap_modes_{ wrap_modes }
    , border_color_{ border_color }
    , layers_{ layers }
    , deallocation_token_{ render_deallocator.insert([this]() { deallocate(); })
}
{
    if (layers == 0) {
        THROW_OR_ABORT("Number of texture layers is zero");
    }
    CHK(glGenTextures(1, &handle_));
}

Texture::Texture(
    GLuint handle,
    TextureTarget target,
    ColorMode color_mode,
    MipmapMode mipmap_mode,
    InterpolationMode magnifying_interpolation_mode,
    FixedArray<WrapMode, 2> wrap_modes,
    const FixedArray<float, 4>& border_color,
    uint32_t layers)
    : handle_{ check_handle(handle) }
    , target_{ target }
    , color_mode_{ color_mode }
    , mipmap_mode_{ mipmap_mode }
    , magnifying_interpolation_mode_{ magnifying_interpolation_mode }
    , wrap_modes_{ wrap_modes }
    , border_color_{ border_color }
    , layers_{ layers }
    , deallocation_token_{ render_deallocator.insert([this]() { deallocate(); }) }
{
    if (layers == 0) {
        THROW_OR_ABORT("Number of texture layers is zero");
    }
}

Texture::Texture(
    GenerateTexture,
    TextureTarget target,
    GLenum format,
    bool with_mipmaps,
    InterpolationMode magnifying_interpolation_mode,
    GLint wrap_s,
	GLint wrap_t,
    const FixedArray<float, 4>& border_color,
    uint32_t layers)
    : Texture{
        generate_texture,
        target,
        format_to_color_mode(format),
        with_mipmaps ? MipmapMode::WITH_MIPMAPS : MipmapMode::NO_MIPMAPS,
        magnifying_interpolation_mode,
        { wrap_mode_from_native(wrap_s), wrap_mode_from_native(wrap_t) },
        border_color,
        layers }
{}

Texture::Texture(
    GLuint handle,
    TextureTarget target,
    GLenum format,
    bool with_mipmaps,
    InterpolationMode magnifying_interpolation_mode,
    GLint wrap_s,
	GLint wrap_t,
    const FixedArray<float, 4>& border_color,
    uint32_t layers)
    : Texture{
        handle,
        target,
        format_to_color_mode(format),
        with_mipmaps ? MipmapMode::WITH_MIPMAPS : MipmapMode::NO_MIPMAPS,
        magnifying_interpolation_mode,
        { wrap_mode_from_native(wrap_s), wrap_mode_from_native(wrap_t) },
        border_color,
        layers }
{}

Texture::Texture(Texture&& other) noexcept
    : handle_{ other.handle_ }
    , color_mode_{ other.color_mode_ }
    , mipmap_mode_{ other.mipmap_mode_ }
    , magnifying_interpolation_mode_{ other.magnifying_interpolation_mode_ }
    , wrap_modes_{ other.wrap_modes_ }
    , border_color_{ other.border_color_ }
    , layers_{ other.layers_ }
    , deallocation_token_{ std::move(other.deallocation_token_) }
{
    other.handle_ = (GLuint)-1;
}

Texture::~Texture() {
    deallocate();
}

void Texture::deallocate() {
    if (handle_ != (GLuint)-1) {
        try_delete_texture(handle_);
    }
}

uint32_t Texture::handle32() const {
    return handle_;
}

uint64_t Texture::handle64() const {
    THROW_OR_ABORT("Unsupported texture handle type");
}

uint32_t& Texture::handle32() {
    return handle_;
}

uint64_t& Texture::handle64() {
    THROW_OR_ABORT("Unsupported texture handle type");
}

bool Texture::texture_is_loaded_and_try_preload() {
    return true;
}

TextureTarget Texture::target() const {
    return target_;
}

ColorMode Texture::color_mode() const {
    return color_mode_;
}

MipmapMode Texture::mipmap_mode() const {
    return mipmap_mode_;
}

InterpolationMode Texture::magnifying_interpolation_mode() const {
    return magnifying_interpolation_mode_;
}

WrapMode Texture::wrap_modes(size_t i) const {
    assert_true(i < 2);
    return wrap_modes_(i);
}

FixedArray<float, 4> Texture::border_color() const {
    return border_color_;
}

uint32_t Texture::layers() const {
    return layers_;
}
