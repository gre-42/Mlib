#pragma once
#include <Mlib/Geometry/Material/Billboard_Atlas_Instance.hpp>
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Material/Depth_Func.hpp>
#include <Mlib/Geometry/Material/Interior_Textures.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Aggregate_Mode.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Transformation_Mode.hpp>
#include <compare>

namespace Mlib {

std::strong_ordering operator <=> (const std::vector<BlendMapTexture>& a, const std::vector<BlendMapTexture>& b);
std::strong_ordering operator <=> (const std::vector<BillboardAtlasInstance>& a, const std::vector<BillboardAtlasInstance>& b);

struct Material {
    // First element to support sorting.
    int continuous_blending_z_order = 0;
    // Second element to support sorting.
    BlendMode blend_mode = BlendMode::OFF;
    // Third element to support sorting.
    DepthFunc depth_func = DepthFunc::LESS;
    bool depth_test = true;
    std::vector<BlendMapTexture> textures;
    std::string dirt_texture;
    InteriorTextures interior_textures;
    ExternalRenderPassType occluded_pass = ExternalRenderPassType::NONE;
    ExternalRenderPassType occluder_pass = ExternalRenderPassType::NONE;
    OrderableFixedArray<float, 4> alpha_distances = { default_distances };
    WrapMode wrap_mode_s = WrapMode::REPEAT;
    WrapMode wrap_mode_t = WrapMode::REPEAT;
    AggregateMode aggregate_mode = AggregateMode::OFF;
    TransformationMode transformation_mode = TransformationMode::ALL;
    std::vector<BillboardAtlasInstance> billboard_atlas_instances;
    size_t number_of_frames = 1;
    OrderableFixedArray<float, 2> distances{ default_distances_hard };
    bool is_small = false;
    bool cull_faces = true;
    bool reorient_uv0 = false;
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
    bool is_small_billboard(uint32_t billboard_id) const;
    std::string identifier() const;
    std::partial_ordering operator <=> (const Material&) const = default;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(continuous_blending_z_order);
        archive(blend_mode);
        archive(depth_func);
        archive(depth_test);
        archive(textures);
        archive(dirt_texture);
        archive(interior_textures);
        archive(occluded_pass);
        archive(occluder_pass);
        archive(alpha_distances);
        archive(wrap_mode_s);
        archive(wrap_mode_t);
        archive(aggregate_mode);
        archive(transformation_mode);
        archive(billboard_atlas_instances);
        archive(number_of_frames);
        archive(distances);
        archive(is_small);
        archive(cull_faces);
        archive(reorient_uv0);
        archive(ambience);
        archive(diffusivity);
        archive(specularity);
        archive(draw_distance_add);
        archive(draw_distance_slop);
        archive(draw_distance_noperations);
    }
};

}
