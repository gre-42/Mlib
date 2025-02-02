#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <memory>
#include <optional>

namespace Mlib {

enum class ExternalRenderPassType;

struct Light {
    FixedArray<float, 3> ambient{1.f, 1.f, 1.f};
    FixedArray<float, 3> diffuse{1.f, 1.f, 1.f};
    FixedArray<float, 3> specular{1.f, 1.f, 1.f};
    FixedArray<float, 3> fresnel_ambient{1.f, 1.f, 1.f};
    FixedArray<float, 3> fog_ambient{1.f, 1.f, 1.f};
    std::shared_ptr<ITextureHandle> lightmap_color;
    std::shared_ptr<ITextureHandle> lightmap_depth;
    std::optional<FixedArray<ScenePos, 4, 4>> vp;
    ExternalRenderPassType shadow_render_pass;
    bool emits_colors() const;
    size_t shading_hash() const;
};

}
