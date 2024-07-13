#pragma once
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Render/Any_Gl.hpp>

namespace Mlib {

class Gl3TextureHandle: public ITextureHandle {
public:
	virtual ~Gl3TextureHandle() override;
	virtual uint32_t handle32() const override;
	virtual uint64_t handle64() const override;
	virtual uint32_t& handle32() override;
	virtual uint64_t& handle64() override;
private:
	GLuint handle_;
};

}
