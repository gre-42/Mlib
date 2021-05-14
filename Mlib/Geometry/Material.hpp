#pragma once
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Material/Occluded_Type.hpp>
#include <Mlib/Geometry/Material/Occluder_Type.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Aggregate_Mode.hpp>
#include <Mlib/Scene_Graph/Transformation_Mode.hpp>
#include <compare>

namespace Mlib {

std::strong_ordering operator <=> (const std::vector<BlendMapTexture>& a, const std::vector<BlendMapTexture>& b);

struct Material {
    // First element to support sorting.
    int continuous_blending_z_order = 0;
    // Second element to support sorting.
    BlendMode blend_mode = BlendMode::OFF;
    std::vector<BlendMapTexture> textures;
    std::string dirt_texture;
    OccludedType occluded_type = OccludedType::OFF;
    OccluderType occluder_type = OccluderType::OFF;
    bool occluded_by_black = true;
    OrderableFixedArray<float, 4> alpha_distances = { default_distances };
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
    Material& compute_color_mode();
    bool has_normalmap() const;
    bool fragments_depend_on_distance() const;
    bool fragments_depend_on_normal() const;
    std::partial_ordering operator <=> (const Material&) const;
};

}
