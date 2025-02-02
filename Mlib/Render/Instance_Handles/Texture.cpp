#include "Texture.hpp"
#include <Mlib/Geometry/Material/Color_Mode.hpp>
#include <Mlib/Geometry/Material/Mipmap_Mode.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Try_Delete.hpp>
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
	ColorMode color_mode,
	MipmapMode mipmap_mode)
	: handle_{ (GLuint)-1 }
	, color_mode_{ color_mode }
	, mipmap_mode_{ mipmap_mode }
	, deallocation_token_{ render_deallocator.insert([this]() { deallocate(); })
}
{
	CHK(glGenTextures(1, &handle_));
}

Texture::Texture(
	GLuint handle,
	ColorMode color_mode,
	MipmapMode mipmap_mode)
	: handle_{ check_handle(handle) }
	, color_mode_{ color_mode }
	, mipmap_mode_{ mipmap_mode }
	, deallocation_token_{ render_deallocator.insert([this]() { deallocate(); }) }
{}

Texture::Texture(
	GenerateTexture,
	GLenum format,
	bool with_mipmaps)
	: Texture{
		generate_texture,
		format_to_color_mode(format),
		with_mipmaps ? MipmapMode::WITH_MIPMAPS : MipmapMode::NO_MIPMAPS }
{}

Texture::Texture(
	GLuint handle,
	GLenum format,
	bool with_mipmaps)
	: Texture{
		handle,
		format_to_color_mode(format),
		with_mipmaps ? MipmapMode::WITH_MIPMAPS : MipmapMode::NO_MIPMAPS }
{}

Texture::Texture(Texture&& other) noexcept
	: handle_{ other.handle_ }
	, color_mode_{ other.color_mode_ }
	, mipmap_mode_{ other.mipmap_mode_ }
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

ColorMode Texture::color_mode() const {
	return color_mode_;
}

MipmapMode Texture::mipmap_mode() const {
	return mipmap_mode_;
}
