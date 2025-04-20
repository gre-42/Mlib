#pragma once
#include <Mlib/Billboard_Id.hpp>
#include <Mlib/Geometry/Material/Aggregate_Mode.hpp>
#include <Mlib/Geometry/Material/Billboard_Atlas_Instance.hpp>
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>
#include <Mlib/Geometry/Material/Blend_Mode.hpp>
#include <Mlib/Geometry/Material/Depth_Func.hpp>
#include <Mlib/Geometry/Material/Interior_Textures.hpp>
#include <Mlib/Geometry/Material/Interpolation_Mode.hpp>
#include <Mlib/Geometry/Material/Render_Pass.hpp>
#include <Mlib/Geometry/Material/Shading.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Geometry/Material/Transformation_Mode.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <compare>
#include <cstddef>
#include <string>
#include <vector>

namespace Mlib {

struct Morphology;

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
    std::vector<BlendMapTexture> textures_color;
    std::vector<BlendMapTexture> textures_alpha;
    float period_world = 0.f;
    VariableAndHash<std::string> reflection_map;
    VariableAndHash<std::string> dirt_texture;
    InteriorTextures interior_textures;
    std::vector<float> continuous_layer_x;
    std::vector<float> continuous_layer_y;
    ExternalRenderPassType occluded_pass = ExternalRenderPassType::NONE;
    ExternalRenderPassType occluder_pass = ExternalRenderPassType::NONE;
    bool contains_skidmarks = false;
    OrderableFixedArray<float, 4> alpha_distances = { default_linear_distances };
    InterpolationMode magnifying_interpolation_mode = InterpolationMode::NEAREST;
    AggregateMode aggregate_mode = AggregateMode::NONE;
    TransformationMode transformation_mode = TransformationMode::ALL;
    std::vector<BillboardAtlasInstance> billboard_atlas_instances;
    size_t number_of_frames = 1;
    bool cull_faces = true;
    bool reorient_uv0 = false;
    Shading shading;
    float alpha = 1.f;
    bool reflect_only_y = false;
    float draw_distance_add = 500;
    float draw_distance_slop = 10;
    size_t draw_distance_noperations = 0;
    bool dynamically_lighted = false;
    Material& compute_color_mode();
    const BillboardAtlasInstance& billboard_atlas_instance(
        BillboardId billboard_id,
        const std::string& name) const;
    ScenePos max_center_distance2(
        BillboardId billboard_id,
        const Morphology& morphology,
        const std::string& name) const;
    ExternalRenderPassType get_occluder_pass(
        BillboardId billboard_id,
        const std::string& name) const;
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
        archive(textures_color);
        archive(textures_alpha);
        archive(period_world);
        archive(reflection_map);
        archive(dirt_texture);
        archive(interior_textures);
        archive(continuous_layer_x);
        archive(continuous_layer_y);
        archive(occluded_pass);
        archive(occluder_pass);
        archive(contains_skidmarks);
        archive(alpha_distances);
        archive(magnifying_interpolation_mode);
        archive(aggregate_mode);
        archive(transformation_mode);
        archive(billboard_atlas_instances);
        archive(number_of_frames);
        archive(cull_faces);
        archive(reorient_uv0);
        archive(shading);
        archive(alpha);
        archive(reflect_only_y);
        archive(draw_distance_add);
        archive(draw_distance_slop);
        archive(draw_distance_noperations);
        archive(dynamically_lighted);
    }
};

}
