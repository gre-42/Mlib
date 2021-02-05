#pragma once
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Material/Occluded_Type.hpp>
#include <Mlib/Geometry/Material/Occluder_Type.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Aggregate_Mode.hpp>
#include <Mlib/Scene_Graph/Transformation_Mode.hpp>
#include <compare>

namespace Mlib {

inline std::strong_ordering operator <=> (const std::vector<BlendMapTexture>& a, const std::vector<BlendMapTexture>& b) {
    if (a.size() < b.size()) {
        return std::strong_ordering::less;
    }
    if (a.size() > b.size()) {
        return std::strong_ordering::greater;
    }
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] < b[i]) {
            return std::strong_ordering::less;
        }
        if (a[i] > b[i]) {
            return std::strong_ordering::greater;
        }
    }
    return std::strong_ordering::equal;
}

struct Material {
    // First element to support sorting.
    int continuous_blending_z_order = 0;
    std::vector<BlendMapTexture> textures;
    std::string dirt_texture;
    OccludedType occluded_type = OccludedType::OFF;
    OccluderType occluder_type = OccluderType::OFF;
    bool occluded_by_black = true;
    BlendMode blend_mode = BlendMode::OFF;
    bool depth_func_equal = false;
    WrapMode wrap_mode_s = WrapMode::REPEAT;
    WrapMode wrap_mode_t = WrapMode::REPEAT;
    bool collide = true;
    AggregateMode aggregate_mode = AggregateMode::OFF;
    TransformationMode transformation_mode = TransformationMode::ALL;
    bool is_small = false;
    bool cull_faces = true;
    OrderableFixedArray<float, 3> ambience{0.5f, 0.5f, 0.5f};
    OrderableFixedArray<float, 3> diffusivity{1.f, 1.f, 1.f};
    OrderableFixedArray<float, 3> specularity{1.f, 1.f, 1.f};
    float draw_distance_add = 500;
    float draw_distance_slop = 10;
    size_t draw_distance_noperations = 0;
    inline Material& compute_color_mode() {
        for (auto& t : textures) {
            t.texture_descriptor.color_mode = (blend_mode == BlendMode::OFF)
                ? ColorMode::RGB
                : ColorMode::RGBA;
        }
        return *this;
    }
    bool has_normalmap() const {
        for (const auto& t : textures) {
            if (!t.texture_descriptor.normal.empty()) {
                return true;
            }
        }
        return false;
    }
    std::partial_ordering operator <=> (const Material&) const = default;
};

}
