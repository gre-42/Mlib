#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <memory>

namespace Mlib {

enum class ParticleType;

struct Skidmark {
    ParticleType particle_type;
    std::shared_ptr<ITextureHandle> texture;
    FixedArray<ScenePos, 4, 4> vp;
    size_t shading_hash() const;
};

}
