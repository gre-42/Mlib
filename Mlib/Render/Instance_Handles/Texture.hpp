#pragma once
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>

namespace Mlib {

static const struct GenerateTexture {} generate_texture;

class Texture: public ITextureHandle {
	Texture(const Texture&) = delete;
	Texture& operator = (const Texture&) = delete;
public:
	explicit Texture(GenerateTexture);
	explicit Texture(GLuint handle);
	Texture(Texture&& other) noexcept;
	~Texture();
	Texture& operator = (GLuint handle);
	virtual uint32_t handle32() const;
	virtual uint64_t handle64() const;
	virtual uint32_t& handle32();
	virtual uint64_t& handle64();
private:
	void deallocate();
	GLuint handle_;
	DeallocationToken deallocation_token_;
};

}
