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
		ColorMode color_mode,
		MipmapMode mipmap_mode,
		FixedArray<WrapMode, 2> wrap_modes);
	Texture(
		GLuint handle,
		ColorMode color_mode,
		MipmapMode mipmap_mode,
		FixedArray<WrapMode, 2> wrap_modes);
	Texture(
		GenerateTexture,
		GLenum format,
		bool with_mipmaps,
		GLint wrap_s,
		GLint wrap_t);
	Texture(
		GLuint handle,
		GLenum format,
		bool with_mipmaps,
		GLint wrap_s,
		GLint wrap_t);
	Texture(Texture&& other) noexcept;
	~Texture();
	virtual uint32_t handle32() const override;
	virtual uint64_t handle64() const override;
	virtual uint32_t& handle32() override;
	virtual uint64_t& handle64() override;
	virtual bool texture_is_loaded_and_try_preload() override;
	virtual ColorMode color_mode() const override;
	virtual MipmapMode mipmap_mode() const override;
	virtual WrapMode wrap_modes(size_t i) const override;
private:
	void deallocate();
	GLuint handle_;
	ColorMode color_mode_;
	MipmapMode mipmap_mode_;
	FixedArray<WrapMode, 2> wrap_modes_;
	DeallocationToken deallocation_token_;
};

}
