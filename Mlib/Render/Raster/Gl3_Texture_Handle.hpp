#pragma once
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Render/Any_Gl.hpp>

namespace Mlib {

class Gl3TextureHandle: public ITextureHandle {
public:
	virtual ~Gl3TextureHandle() override;
};

}
