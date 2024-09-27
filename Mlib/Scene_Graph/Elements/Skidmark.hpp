#pragma once
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <memory>

namespace Mlib {

struct Skidmark {
    std::shared_ptr<ITextureHandle> texture;
    FixedArray<ScenePos, 4, 4> vp;
};

}
