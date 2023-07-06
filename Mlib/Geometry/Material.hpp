#pragma once
#include <Mlib/Geometry/Material/Billboard_Atlas_Instance.hpp>
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Material/Depth_Func.hpp>
#include <Mlib/Geometry/Material/Interior_Textures.hpp>
#include <Mlib/Geometry/Material/Interpolation_Mode.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Aggregate_Mode.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Transformation_Mode.hpp>
#include <compare>

namespace Mlib {

/** Material with included sorting support for later rendering.
 *
 * Notes about sorting:
 *
 * Comparing two materials can be a time-consuming process because it
 * includes comparing texture names and more.
 * Therefore, a faster comparison function "rendering_sorting_key" is provided.
 * The slow, complete sorting is used in the "AggregateArrayRender", while the
 * faster comparison is used in the ArrayInstancesRenderer.
 * 
 * The material's sorting key only sorts the vertex arrays, not instances.
 * Instances are sorted using "VisibilityCheck::sorting_key".
 * "Blended::sorting_key" only affects blended, non-instanced or aggregated objects
 * and does not use the absolute value because it is updated in each frame.
 */
struct Material {
    // First element to support sorting.
    BlendMode blend_mode = BlendMode::OFF;
    // Second element to support sorting.
    // As the name suggests, this flag shall only affect
    // continuously blended materials and therefore
    // has a lower priority than the blending mode.
    int continuous_blending_z_order = 0;
    // Third element to support sorting.
    DepthFunc depth_func = DepthFunc::LESS;
    bool depth_test = true;
    std::vector<BlendMapTexture> textures;
    std::string reflection_map;
    std::string dirt_texture;
    InteriorTextures interior_textures;
    ExternalRenderPassType occluded_pass = ExternalRenderPassType::NONE;
    ExternalRenderPassType occluder_pass = ExternalRenderPassType::NONE;
    OrderableFixedArray<float, 4> alpha_distances = { default_linear_distances };
    WrapMode wrap_mode_s = WrapMode::REPEAT;
    WrapMode wrap_mode_t = WrapMode::REPEAT;
    InterpolationMode magnifying_interpolation_mode = InterpolationMode::NEAREST;
    AggregateMode aggregate_mode = AggregateMode::NONE;
    TransformationMode transformation_mode = TransformationMode::ALL;
    std::vector<BillboardAtlasInstance> billboard_atlas_instances;
    size_t number_of_frames = 1;
    OrderableFixedArray<float, 2> center_distances{ default_step_distances };
    float max_triangle_distance = INFINITY;
    bool cull_faces = true;
    bool reorient_uv0 = false;
    OrderableFixedArray<float, 3> emissivity{0.f, 0.f, 0.f};
    OrderableFixedArray<float, 3> ambience{0.5f, 0.5f, 0.5f};
    OrderableFixedArray<float, 3> diffusivity{1.f, 1.f, 1.f};
    OrderableFixedArray<float, 3> specularity{1.f, 1.f, 1.f};
    float alpha = 1.f;
    bool reflect_only_y = false;
    float draw_distance_add = 500;
    float draw_distance_slop = 10;
    size_t draw_distance_noperations = 0;
    Material& compute_color_mode();
    bool has_normalmap() const;
    bool fragments_depend_on_distance() const;
    bool fragments_depend_on_normal() const;
    const BillboardAtlasInstance& billboard_atlas_instance(uint32_t billboard_id) const;
    double max_center_distance(uint32_t billboard_id) const;
    ExternalRenderPassType get_occluder_pass(uint32_t billboard_id) const;
    std::string identifier() const;
    inline auto rendering_sorting_key() const {
        return std::make_tuple(blend_mode, continuous_blending_z_order, depth_func);
    }
    std::partial_ordering operator <=> (const Material&) const = default;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(blend_mode);
        archive(continuous_blending_z_order);
        archive(depth_func);
        archive(depth_test);
        archive(textures);
        archive(reflection_map);
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
        archive(center_distances);
        archive(cull_faces);
        archive(reorient_uv0);
        archive(emissivity);
        archive(ambience);
        archive(diffusivity);
        archive(specularity);
        archive(reflect_only_y);
        archive(draw_distance_add);
        archive(draw_distance_slop);
        archive(draw_distance_noperations);
    }
};

}
