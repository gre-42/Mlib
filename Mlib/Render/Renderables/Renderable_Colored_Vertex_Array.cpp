#include "Renderable_Colored_Vertex_Array.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Mesh/Bone.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Transformed_Colored_Vertex_Array.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Colored_Render_Program.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Render/Resources/Substitution_Info.hpp>
#include <Mlib/Render/Toggle_Benchmark_Rendering.hpp>
#include <Mlib/Scene_Graph/Elements/Animation_State.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Light.hpp>
#include <Mlib/Scene_Graph/Instances/Large_Instances_Queue.hpp>
#include <Mlib/Scene_Graph/Instances/Small_Instances_Queues.hpp>
#include <Mlib/Scene_Graph/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Visibility_Check.hpp>
#include <Mlib/Strings/String.hpp>
#include <climits>

// #undef LOG_FUNCTION
// #undef LOG_INFO
// #define LOG_FUNCTION(msg) ::Mlib::Log log(msg)
// #define LOG_INFO(msg) log.info(msg)

using namespace Mlib;

struct TextureIndexCalculator {
    size_t ntextures_color;
    size_t ntextures_filtered_lights;
    size_t ntextures_normal;
    size_t ntextures_dirt;
    size_t ntextures_interior;
    size_t ntextures_reflection;
    size_t ntextures_specular;

    size_t id_color(size_t i) {
        return i;
    }
    size_t id_light(size_t i) {
        return ntextures_color + i;
    }
    size_t id_normal(size_t i) {
        return id_light(0) + ntextures_filtered_lights + i;
    }
    size_t id_dirt(size_t i) {
        return id_normal(0) + ntextures_normal + i;
    }
    size_t id_reflection() {
        return id_dirt(0) + ntextures_dirt;
    }
    size_t id_interior(size_t i) {
        return id_reflection() + ntextures_reflection + i;
    }
    size_t id_specular() {
        return id_interior(0) + ntextures_interior;
    }
};

static const int CONTINUOUS_BLENDING_Z_ORDER_UNDEFINED = INT_MAX;
static const int CONTINUOUS_BLENDING_Z_ORDER_CONFLICTING = INT_MIN;

RenderableColoredVertexArray::RenderableColoredVertexArray(
    const std::shared_ptr<const ColoredVertexArrayResource>& rcva,
    const RenderableResourceFilter& renderable_resource_filter)
: rcva_{rcva},
  continuous_blending_z_order_{CONTINUOUS_BLENDING_Z_ORDER_UNDEFINED},
  secondary_rendering_resources_{RenderingContextStack::rendering_resources()}
{
#ifdef DEBUG
    rcva_->triangles_res_->check_consistency();
#endif
    requires_blending_pass_ = false;
    auto add_cvas = [&]<typename TPos>(const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas)
    {
        size_t i = 0;
        for (const auto& t : cvas) {
            if (renderable_resource_filter.matches(i++, *t)) {
                if ((t->material.aggregate_mode == AggregateMode::NONE) ||
                    (rcva->instances_ != nullptr))
                {
                    if constexpr (std::is_same_v<TPos, float>) {
                        aggregate_off_.push_back(t);
                        required_occluder_passes_.insert(t->material.occluder_pass);
                    } else {
                        throw std::runtime_error("Instances and aggregate=off require single precision (material: " + t->material.identifier() + ')');
                    }
                } else if (t->material.aggregate_mode == AggregateMode::ONCE) {
                    if constexpr (std::is_same_v<TPos, double>) {
                        aggregate_once_.push_back(t);
                    } else {
                        throw std::runtime_error("Aggregate=once requires double precision (material: " + t->material.identifier() + ')');
                    }
                } else if (t->material.aggregate_mode == AggregateMode::SORTED_CONTINUOUSLY) {
                    if constexpr (std::is_same_v<TPos, double>) {
                        aggregate_sorted_continuously_.push_back(t);
                    } else {
                        throw std::runtime_error("Aggregate=sorted_continuously requires double precision (material: " + t->material.identifier() + ')');
                    }
                } else if (t->material.aggregate_mode == AggregateMode::INSTANCES_ONCE) {
                    if constexpr (std::is_same_v<TPos, float>) {
                        instances_once_.push_back(t);
                    } else {
                        throw std::runtime_error("Aggregate=instances_once requires single precision (material: " + t->material.identifier() + ')');
                    }
                } else if (t->material.aggregate_mode == AggregateMode::INSTANCES_SORTED_CONTINUOUSLY) {
                    if constexpr (std::is_same_v<TPos, float>) {
                        instances_sorted_continuously_.push_back(t);
                    } else {
                        throw std::runtime_error("Aggregate=instances_sorted_continuously requires single precision (material: " + t->material.identifier() + ')');
                    }
                } else {
                    throw std::runtime_error("Unknown aggregate mode");
                }
                if ((t->material.continuous_blending_z_order == CONTINUOUS_BLENDING_Z_ORDER_UNDEFINED) ||
                    (t->material.continuous_blending_z_order == CONTINUOUS_BLENDING_Z_ORDER_CONFLICTING))
                {
                    throw std::runtime_error("Unsupported \"continuous_blending_z_order\" value");
                }
                if (continuous_blending_z_order_ != CONTINUOUS_BLENDING_Z_ORDER_CONFLICTING) {
                    if ((t->material.blend_mode == BlendMode::CONTINUOUS) &&
                        (t->material.aggregate_mode == AggregateMode::NONE))
                    {
                        requires_blending_pass_ = true;
                        if (continuous_blending_z_order_ == CONTINUOUS_BLENDING_Z_ORDER_UNDEFINED) {
                            continuous_blending_z_order_ = t->material.continuous_blending_z_order;
                        } else if (continuous_blending_z_order_ != t->material.continuous_blending_z_order) {
                            continuous_blending_z_order_ = CONTINUOUS_BLENDING_Z_ORDER_CONFLICTING;
                        }
                    }
                }
            }
        }
    };
    add_cvas(rcva->triangles_res_->scvas);
    add_cvas(rcva->triangles_res_->dcvas);
}

RenderableColoredVertexArray::~RenderableColoredVertexArray()
{}

GLint get_wrap_param(WrapMode mode) {
    switch(mode) {
    case WrapMode::REPEAT:
        return GL_REPEAT;
    case WrapMode::CLAMP_TO_EDGE:
        return GL_CLAMP_TO_EDGE;
    case WrapMode::CLAMP_TO_BORDER:
        return GL_CLAMP_TO_BORDER;
    default:
        throw std::runtime_error("Unknown wrap mode");
    }
}

std::vector<OffsetAndQuaternion<float, float>> RenderableColoredVertexArray::calculate_absolute_bone_transformations(const AnimationState* animation_state) const
{
    TIME_GUARD_DECLARE(time_guard, "calculate_absolute_bone_transformations", "calculate_absolute_bone_transformations");
    if (!rcva_->triangles_res_->bone_indices.empty()) {
        if (animation_state == nullptr) {
            throw std::runtime_error("Animation without animation state");
        }
        auto get_abt = [this](const std::string& animation_name, const AnimationFrame& animation_frame) {
            if (animation_name.empty()) {
                throw std::runtime_error("Animation frame has no name");
            }
            if (std::isnan(animation_frame.time)) {
                throw std::runtime_error("Vertex array loop time is NAN");
            }
            auto poses = rcva_->scene_node_resources_.get_relative_poses(
                animation_name,
                animation_frame.time);
            std::vector<OffsetAndQuaternion<float, float>> ms = rcva_->triangles_res_->vectorize_joint_poses(poses);
            std::vector<OffsetAndQuaternion<float, float>> absolute_bone_transformations = rcva_->triangles_res_->skeleton->rebase_to_initial_absolute_transform(ms);
            if (absolute_bone_transformations.size() != rcva_->triangles_res_->bone_indices.size()) {
                throw std::runtime_error("Number of bone indices differs from number of quaternions");
            }
            return absolute_bone_transformations;
        };
        if (animation_state->aperiodic_animation_frame.active()) {
            return get_abt(animation_state->aperiodic_skelletal_animation_name, animation_state->aperiodic_animation_frame.frame);
        } else {
            return get_abt(animation_state->periodic_skelletal_animation_name, animation_state->periodic_skelletal_animation_frame.frame);
        }
    } else {
        return {};
    }
}

void RenderableColoredVertexArray::render_cva(
    const std::shared_ptr<ColoredVertexArray<float>>& cva,
    const std::vector<OffsetAndQuaternion<float, float>>& absolute_bone_transformations,
    const FixedArray<double, 4, 4>& mvp,
    const TransformationMatrix<float, double, 3>& m,
    const TransformationMatrix<float, double, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const RenderPass& render_pass,
    const AnimationState* animation_state,
    const ColorStyle* color_style) const
{
    LOG_FUNCTION("render_cva");
    LOG_INFO("RenderableColoredVertexArray::render_cva " + cva->identifier());
    TIME_GUARD_DECLARE(time_guard, "render_cva", cva->identifier());
    // std::cerr << external_render_pass_type_to_string(render_pass.external.pass) << " " << cva->identifier();
    // if (rcva_->instances_ != nullptr) {
    //     std::cerr << ", #inst: " << rcva_->instances_->size();
    // }
    // This check passes because the arrays are filtered in the constructor.
    assert_true((cva->material.aggregate_mode == AggregateMode::NONE) || (rcva_->instances_ != nullptr));
    if (render_pass.internal == InternalRenderPass::INITIAL && cva->material.blend_mode == BlendMode::CONTINUOUS) {
        // std::cerr << ", skipped (0)" << std::endl;
        return;
    }
    if (render_pass.internal == InternalRenderPass::BLENDED && cva->material.blend_mode != BlendMode::CONTINUOUS) {
        // std::cerr << ", skipped (1)" << std::endl;
        return;
    }
    // This is now done in the VisibilityCheck class.
    // if (render_pass.external.pass == ExternalRenderPassType::LIGHTMAP_TO_TEXTURE && render_pass.external.black_node_name.empty() && cva->material.occluder_pass == OccluderType::OFF) {
    //     return;
    // }
    VisibilityCheck vc{mvp};
    if (rcva_->instances_ == nullptr) {
        if (!vc.is_visible(cva->material, UINT32_MAX, scene_graph_config, render_pass.external.pass))
        {
            // std::cerr << ", skipped (2)" << std::endl;
            return;
        }
    } else if (!VisibilityCheck::instances_are_visible(cva->material, render_pass.external.pass)) {
        return;
    }
    // std::cerr << std::endl;

    std::vector<std::pair<TransformationMatrix<float, double, 3>, Light*>> filtered_lights;
    std::vector<size_t> lightmap_indices;
    std::vector<size_t> light_noshadow_indices;
    std::vector<size_t> light_shadow_indices;
    std::vector<size_t> black_shadow_indices;
    bool color_requires_normal = !cva->material.diffusivity.all_equal(0) || !cva->material.specularity.all_equal(0);
    bool is_lightmap = any(render_pass.external.pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK);
    if (!is_lightmap && (!cva->material.ambience.all_equal(0) || !cva->material.diffusivity.all_equal(0) || !cva->material.specularity.all_equal(0))) {
        filtered_lights.reserve(lights.size());
        light_noshadow_indices.reserve(lights.size());
        light_shadow_indices.reserve(lights.size());
        black_shadow_indices.reserve(lights.size());
        lightmap_indices.reserve(lights.size());
        {
            size_t i = 0;
            for (const auto& l : lights) {
                // By this definition, objects are occluded/lighted (occluded_pass)
                // by several shadowmaps/lightmaps (low-resolution and high-resolution shadowmaps).
                // The occluder_pass is checked in the "VisibilityCheck" class.
                if (cva->material.occluded_pass < l.second->shadow_render_pass) {
                    continue;
                }
                bool light_emits_colors =
                    (l.second->shadow_render_pass == ExternalRenderPassType::NONE) ||
                    any(l.second->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_EMITS_COLORS_MASK);
                bool light_casts_shadows = any(l.second->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK);

                if (!light_emits_colors && !light_casts_shadows) {
                    continue;
                }
                filtered_lights.push_back(l);
                if (light_emits_colors) {
                    if (light_casts_shadows) {
                        lightmap_indices.push_back(i);
                        light_shadow_indices.push_back(i++);
                        if (l.second->resource_suffix.empty()) {
                            throw std::runtime_error("Light with shadows has no resource suffix");
                        }
                    } else {
                        light_noshadow_indices.push_back(i++);
                        if (!l.second->resource_suffix.empty()) {
                            throw std::runtime_error("Light without shadow has a resource suffix: \"" + l.second->resource_suffix + '"');
                        }
                    }
                } else {
                    lightmap_indices.push_back(i);
                    black_shadow_indices.push_back(i++);
                    if (l.second->resource_suffix.empty()) {
                        throw std::runtime_error("Black shadow has no resource suffix");
                    }
                }
            }
        }
    }
    std::vector<BlendMapTexture*> blended_textures(cva->material.textures.size());
    for (size_t i = 0; i < blended_textures.size(); ++i) {
        blended_textures[i] = &cva->material.textures[i];
    }
    {
        size_t i = 0;
        for (const auto& t : cva->material.textures) {
            if (t.texture_descriptor.color.empty()) {
                throw std::runtime_error("Empty color texture not supported, cva: " + cva->name);
            }
            if (t.texture_descriptor.color_mode == ColorMode::UNDEFINED) {
                throw std::runtime_error("Material's color texture \"" + t.texture_descriptor.color + "\" has undefined color mode");
            }
            if (i == 0) {
                if ((cva->material.blend_mode == BlendMode::OFF) && (t.texture_descriptor.color_mode == ColorMode::RGBA)) {
                    throw std::runtime_error("Opaque material's color texture \"" + t.texture_descriptor.color + "\" was loaded as RGBA");
                }
                if ((cva->material.blend_mode != BlendMode::OFF) && (t.texture_descriptor.color_mode == ColorMode::RGB)) {
                    throw std::runtime_error("Transparent material's color texture \"" + t.texture_descriptor.color + "\" was not loaded as RGB");
                }
            }
            ++i;
        }
    }
    FixedArray<float, 3> emissivity;
    FixedArray<float, 3> ambience;
    FixedArray<float, 3> diffusivity;
    FixedArray<float, 3> specularity;
    if (!is_lightmap) {
        emissivity = color_style && !all(color_style->emissivity == -1.f) ? color_style->emissivity : cva->material.emissivity;
    } else {
        emissivity = 1.f;
    }
    if (!filtered_lights.empty() && !is_lightmap) {
        ambience = color_style && !all(color_style->ambience == -1.f) ? color_style->ambience * cva->material.ambience : cva->material.ambience;
        diffusivity = color_style && !all(color_style->diffusivity == -1.f) ? color_style->diffusivity * cva->material.diffusivity : cva->material.diffusivity;
        specularity = color_style && !all(color_style->specularity == -1.f) ? color_style->specularity * cva->material.specularity : cva->material.specularity;
    } else {
        ambience = 0.f;
        diffusivity = 0.f;
        specularity = 0.f;
    }
    if (filtered_lights.size() == 1) {
        ambience *= (filtered_lights.front().second->ambience != 0.f).casted<float>();
        diffusivity *= (filtered_lights.front().second->diffusivity != 0.f).casted<float>();
        specularity *= (filtered_lights.front().second->specularity != 0.f).casted<float>();
    }
    TextureIndexCalculator tic;
    tic.ntextures_color = (
        !is_lightmap ||
        ((cva->material.blend_mode != BlendMode::OFF) && (cva->material.depth_func != DepthFunc::EQUAL)))
            ? cva->material.textures.size()
            : 0;
    tic.ntextures_filtered_lights = filtered_lights.size();
    std::vector<size_t> lightmap_indices_color = any(cva->material.occluded_pass & ExternalRenderPassType::LIGHTMAP_COLOR_MASK) ? lightmap_indices : std::vector<size_t>{};
    std::vector<size_t> lightmap_indices_depth = any(cva->material.occluded_pass & ExternalRenderPassType::LIGHTMAP_DEPTH_MASK) ? lightmap_indices : std::vector<size_t>{};
    if (is_lightmap || cva->material.textures.empty() || filtered_lights.empty() || (all(specularity == 0.f) && cva->material.reflection_map.empty())) {
        tic.ntextures_specular = 0;
    } else if (cva->material.textures.size() == 1) {
        tic.ntextures_specular = !cva->material.textures[0].texture_descriptor.specular.empty();
    } else {
        for (const auto& t : cva->material.textures) {
            if (!t.texture_descriptor.specular.empty()) {
                throw std::runtime_error("Specular maps not supported for blended textures");
            }
        }
        tic.ntextures_specular = 0;
    }
    tic.ntextures_normal = !filtered_lights.empty() && color_requires_normal && render_config.normalmaps && cva->material.has_normalmap() && !is_lightmap ? cva->material.textures.size() : 0;
    tic.ntextures_reflection = (size_t)(!is_lightmap && !cva->material.reflection_map.empty());
    tic.ntextures_dirt = (!cva->material.dirt_texture.empty()) && !is_lightmap ? 2 : 0;
    tic.ntextures_interior = (!cva->material.interior_textures.empty()) && !is_lightmap ? INTERIOR_COUNT : 0;
    bool has_instances = (rcva_->instances_ != nullptr);
    bool has_lookat = (cva->material.transformation_mode == TransformationMode::POSITION_LOOKAT);
    bool has_yangle = (cva->material.transformation_mode == TransformationMode::POSITION_YANGLE);
    OrderableFixedArray<float, 4> alpha_distances;
    bool fragments_depend_on_distance;
    if (is_lightmap) {
        fragments_depend_on_distance = false;
        alpha_distances = default_linear_distances;
    } else {
        fragments_depend_on_distance = cva->material.fragments_depend_on_distance();
        alpha_distances = cva->material.alpha_distances;
    }
    bool fragments_depend_on_normal =
        !is_lightmap &&
        (cva->material.fragments_depend_on_normal() || (tic.ntextures_interior != 0));
    if ((tic.ntextures_color == 0) && (tic.ntextures_dirt != 0)) {
        throw std::runtime_error(
            "Combination of ((ntextures_color == 0) && (ntextures_dirt != 0)) is not supported. Textures: " +
            join(" ", cva->material.textures, [](const auto& v) { return v.texture_descriptor.color; }));
    }
    std::string reflection_map;
    float reflection_strength = 0.f;
    if (!is_lightmap && !cva->material.reflection_map.empty()) {
        if (color_style == nullptr) {
            throw std::runtime_error("cva " + cva->name + ": Material with reflection map \"" + cva->material.reflection_map + "\" has no style");
        }
        auto it = color_style->reflection_maps.find(cva->material.reflection_map);
        if (it == color_style->reflection_maps.end()) {
            throw std::runtime_error(
                "cva " + cva->name + ": Could not find reflection map \""
                + cva->material.reflection_map
                + "\" in style with keys:"
                + join(", ", color_style->reflection_maps, [](const auto& s){return s.first;}));
        }
        reflection_map = it->second;
        reflection_strength = color_style->reflection_strength;
        if (reflection_strength == 0.f) {
            throw std::runtime_error("Reflection strength cannot be zero");
        }
    }
    bool reorient_normals = !cva->material.cull_faces && (any(diffusivity != 0.f) || any(specularity != 0.f));
    if (cva->material.cull_faces && cva->material.reorient_uv0) {
        throw std::runtime_error("reorient_uv0 requires disabled face culling");
    }
    bool reorient_uv0 = cva->material.reorient_uv0 && !is_lightmap;
    LOG_INFO("RenderableColoredVertexArray::render_cva get_render_program");
    assert_true(cva->material.number_of_frames > 0);
    const ColoredRenderProgram& rp = rcva_->get_render_program(
        RenderProgramIdentifier{
            .render_pass = render_pass.external.pass,
            .nlights = filtered_lights.size(),
            .nbones = rcva_->triangles_res_->bone_indices.size(),
            .blend_mode = any(render_pass.external.pass & ExternalRenderPassType::LIGHTMAP_BLOBS_MASK)
                ? BlendMode::CONTINUOUS
                : cva->material.blend_mode,
            .alpha_distances = alpha_distances,
            .ntextures_color = tic.ntextures_color,
            .ntextures_normal = tic.ntextures_normal,
            .lightmap_indices_color = lightmap_indices_color,
            .lightmap_indices_depth = lightmap_indices_depth,
            .has_specularmap = (tic.ntextures_specular != 0),
            .reflection_strength = reflection_strength,
            .reflect_only_y = cva->material.reflect_only_y,
            .ntextures_reflection = tic.ntextures_reflection,
            .ntextures_dirt = tic.ntextures_dirt,
            .ntextures_interior = tic.ntextures_interior,
            .facade_edge_size = cva->material.interior_textures.facade_edge_size,
            .facade_inner_size = cva->material.interior_textures.facade_inner_size,
            .interior_size = cva->material.interior_textures.interior_size,
            .dirt_color_mode = (tic.ntextures_dirt != 0)
                ? rcva_->rendering_resources_->get_existing_texture_descriptor(cva->material.dirt_texture).color_mode
                : ColorMode::UNDEFINED,
            .has_instances = has_instances,
            .has_lookat = has_lookat,
            .has_yangle = has_yangle,
            .has_uv_offset_u = (cva->material.number_of_frames != 1),  // Texture is required in lightmap also due to alpha channel.
            .nbillboard_ids = (uint32_t)cva->material.billboard_atlas_instances.size(),  // Texture is required in lightmap also due to alpha channel.
            .reorient_normals = reorient_normals,
            .reorient_uv0 = reorient_uv0,
            .emissivity = OrderableFixedArray{emissivity},
            .ambience = OrderableFixedArray{ambience},
            .diffusivity = OrderableFixedArray{diffusivity},
            .specularity = OrderableFixedArray{specularity},
            .alpha = cva->material.alpha,
            .orthographic = vc.orthographic(),
            .fragments_depend_on_distance = fragments_depend_on_distance,
            .fragments_depend_on_normal = fragments_depend_on_normal,
            // Not using NAN for ordering.
            .dirtmap_offset = (tic.ntextures_dirt != 0) ? secondary_rendering_resources_->get_offset("dirtmap") : -1234,
            .dirtmap_discreteness = (tic.ntextures_dirt != 0) ? secondary_rendering_resources_->get_discreteness("dirtmap") : -1234,
            .dirt_scale = (tic.ntextures_dirt != 0) ? secondary_rendering_resources_->get_scale("dirtmap") : -1234},
        filtered_lights,
        lightmap_indices,
        light_noshadow_indices,
        light_shadow_indices,
        black_shadow_indices,
        blended_textures);
    LOG_INFO("RenderableColoredVertexArray::render_cva glUseProgram");
    CHK(glUseProgram(rp.program));
    LOG_INFO("RenderableColoredVertexArray::render_cva mvp");
    CHK(glUniformMatrix4fv(rp.mvp_location, 1, GL_TRUE, mvp.casted<float>().flat_begin()));
    if (cva->material.number_of_frames != 1) {
        float uv_offset_u;
        if ((animation_state != nullptr) &&
            !animation_state->aperiodic_animation_frame.frame.is_nan())
        {
            if (animation_state->aperiodic_animation_frame.frame.begin == animation_state->aperiodic_animation_frame.frame.end) {
                uv_offset_u = animation_state->aperiodic_animation_frame.frame.time;
            } else {
                uv_offset_u =
                    (animation_state->aperiodic_animation_frame.frame.time - animation_state->aperiodic_animation_frame.frame.begin) /
                    (animation_state->aperiodic_animation_frame.frame.end - animation_state->aperiodic_animation_frame.frame.begin);
                uv_offset_u = std::round(uv_offset_u * cva->material.number_of_frames) / (float)cva->material.number_of_frames;
            }
        } else {
            uv_offset_u = 0;
        }
        CHK(glUniform1f(rp.uv_offset_u_location, uv_offset_u));
    }
    if (!cva->material.billboard_atlas_instances.empty()) {
        size_t n = cva->material.billboard_atlas_instances.size();
        std::vector<FixedArray<float, 2>> vertex_scale(n);
        std::vector<FixedArray<float, 2>> uv_scale(n);
        std::vector<FixedArray<float, 2>> uv_offset(n);
        std::vector<FixedArray<float, 4>> alpha_distances;
        if (!vc.orthographic()) {
            alpha_distances.resize(n);
        }
        for (size_t i = 0; i < n; ++i) {
            uv_offset[i] = cva->material.billboard_atlas_instances[i].uv_offset;
            uv_scale[i] = cva->material.billboard_atlas_instances[i].uv_scale;
            vertex_scale[i] = cva->material.billboard_atlas_instances[i].vertex_scale;
            if (!vc.orthographic()) {
                alpha_distances[i] = cva->material.billboard_atlas_instances[i].alpha_distances;
            }
        }
        CHK(glUniform2fv(rp.uv_offset_location, n, (const GLfloat*)uv_offset.data()));
        CHK(glUniform2fv(rp.uv_scale_location, n, (const GLfloat*)uv_scale.data()));
        CHK(glUniform2fv(rp.vertex_scale_location, n, (const GLfloat*)vertex_scale.data()));
        if (!vc.orthographic()) {
            CHK(glUniform4fv(rp.alpha_distances_location, n, (const GLfloat*)alpha_distances.data()));
        }
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva textures");
    for (size_t i = 0; i < tic.ntextures_color; ++i) {
        CHK(glUniform1i(rp.texture_color_locations.at(i), (GLint)tic.id_color(i)));
    }
    assert_true(lightmap_indices_color.empty() || lightmap_indices_depth.empty());
    if (!lightmap_indices_color.empty()) {
        for (size_t i : lightmap_indices) {
            CHK(glUniform1i(rp.texture_lightmap_color_locations.at(i), (GLint)tic.id_light(i)));
        }
    }
    if (!lightmap_indices_depth.empty()) {
        for (size_t i : lightmap_indices) {
            CHK(glUniform1i(rp.texture_lightmap_depth_locations.at(i), (GLint)tic.id_light(i)));
        }
    }
    if (tic.ntextures_normal != 0) {
        size_t i = 0;
        for (const auto& t : cva->material.textures) {
            if (!t.texture_descriptor.normal.empty()) {
                CHK(glUniform1i(rp.texture_normalmap_locations.at(i), (GLint)tic.id_normal(i)));
            }
            ++i;
        }
    }
    if (tic.ntextures_reflection != 0) {
        CHK(glUniform1i(rp.texture_reflection_location, (GLint)tic.id_reflection()));
    }
    if (tic.ntextures_dirt != 0) {
        CHK(glUniform1i(rp.texture_dirtmap_location, (GLint)tic.id_dirt(0)));
        CHK(glUniform1i(rp.texture_dirt_location, (GLint)tic.id_dirt(1)));
    }
    if (tic.ntextures_interior != 0) {
        for (size_t i = 0; i < INTERIOR_COUNT; ++i) {
            CHK(glUniform1i(rp.texture_interiormap_location(i), (GLint)tic.id_interior(i)));
        }
    }
    if (tic.ntextures_specular != 0) {
        CHK(glUniform1i(rp.texture_specularmap_location, (GLint)tic.id_specular()));
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva lights");
    {
        bool light_dir_required = (any(diffusivity != 0.f) || any(specularity != 0.f));
        if (light_dir_required || fragments_depend_on_distance || fragments_depend_on_normal || (tic.ntextures_interior != 0)) {
            // CHK(glUniform3fv(rp.light_position_location, 1, t3_from_4x4(filtered_lights.front().first).flat_begin()));
            if (light_dir_required) {
                size_t i = 0;
                for (const auto& l : filtered_lights) {
                    if (!any(l.second->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK)) {
                        auto mz = m.irotate(z3_from_3x3(l.first.R()));
                        mz /= std::sqrt(sum(squared(mz)));
                        CHK(glUniform3fv(rp.light_dir_locations.at(i), 1, mz.flat_begin()));
                    }
                    ++i;
                }
            }
        }
    }
    {
        size_t i = 0;
        for (const auto& l : filtered_lights) {
            if (any(ambience != 0.f) && !any(l.second->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK)) {
                CHK(glUniform3fv(rp.light_ambiences.at(i), 1, l.second->ambience.flat_begin()));
            }
            if (any(diffusivity != 0.f) && !any(l.second->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK)) {
                CHK(glUniform3fv(rp.light_diffusivities.at(i), 1, l.second->diffusivity.flat_begin()));
            }
            if (any(specularity != 0.f) && !any(l.second->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_IS_BLACK_MASK)) {
                CHK(glUniform3fv(rp.light_specularities.at(i), 1, l.second->specularity.flat_begin()));
            }
            ++i;
        }
    }
    {
        bool pred0 = has_lookat || any(specularity != 0.f) || (reflection_strength != 0.f) || (fragments_depend_on_distance && !vc.orthographic());
        if (pred0 || (tic.ntextures_interior != 0)) {
            bool ortho = vc.orthographic();
            auto miv = m.inverted() * iv;
            if (pred0 && ortho) {
                auto d = z3_from_3x3(miv.R());
                d /= std::sqrt(sum(squared(d)));
                CHK(glUniform3fv(rp.view_dir, 1, d.flat_begin()));
            }
            if ((pred0 && !ortho) || (tic.ntextures_interior != 0)) {
                CHK(glUniform3fv(rp.view_pos, 1, miv.t().casted<float>().flat_begin()));
            }
        }
    }
    if (reflection_strength != 0.f) {
        CHK(glUniformMatrix3fv(rp.r_location, 1, GL_TRUE, m.R().T().flat_begin()));
    }
    if (!rcva_->triangles_res_->bone_indices.empty()) {
        size_t i = 0;
        for (const auto& l : absolute_bone_transformations) {
            CHK(glUniform3fv(rp.pose_positions.at(i), 1, l.offset().flat_begin()));
            CHK(glUniform4fv(rp.pose_quaternions.at(i), 1, l.quaternion().vector().flat_begin()));
            ++i;
        }
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva bind texture");
    auto setup_texture = [&cva, &render_pass]() {
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, get_wrap_param(cva->material.wrap_mode_s)));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, get_wrap_param(cva->material.wrap_mode_t)));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
        if (any(render_pass.external.pass & ExternalRenderPassType::LIGHTMAP_BLOBS_MASK)) {
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        } else {
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        }
    };
    if (tic.ntextures_color != 0) {
        size_t i = 0;
        for (const auto& t : cva->material.textures) {
            LOG_INFO("RenderableColoredVertexArray::render_cva get texture \"" + t.texture_descriptor.color + '"');
            GLuint texture = secondary_rendering_resources_->contains_texture(t.texture_descriptor.color)
                ? secondary_rendering_resources_->get_texture(t.texture_descriptor)
                : rcva_->rendering_resources_->get_texture(t.texture_descriptor);
            LOG_INFO("RenderableColoredVertexArray::render_cva bind texture \"" + t.texture_descriptor.color + '"');
            CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_color(i))));
            CHK(glBindTexture(GL_TEXTURE_2D, texture));
            LOG_INFO("RenderableColoredVertexArray::render_cva clamp texture \"" + t.texture_descriptor.color + '"');
            setup_texture();
            CHK(glActiveTexture(GL_TEXTURE0));
            ++i;
        }
    }
    assert_true(lightmap_indices_color.empty() || lightmap_indices_depth.empty());
    LOG_INFO("RenderableColoredVertexArray::render_cva bind light color textures");
    if (!lightmap_indices_color.empty()) {
        for (size_t i : lightmap_indices) {
            std::string mname = "lightmap_color." + filtered_lights.at(i).second->resource_suffix;
            const auto& light_vp = secondary_rendering_resources_->get_vp(mname);
            auto mvp_light = dot2d(light_vp, m.affine());
            CHK(glUniformMatrix4fv(rp.mvp_light_locations.at(i), 1, GL_TRUE, mvp_light.casted<float>().flat_begin()));
            
            CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_light(i))));
            CHK(glBindTexture(GL_TEXTURE_2D, secondary_rendering_resources_->get_texture({.color = mname, .color_mode = ColorMode::RGB})));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
            float border_brightness = 1.f - any(filtered_lights.at(i).second->shadow_render_pass & ExternalRenderPassType::LIGHTMAP_BLOBS_MASK);
            float borderColor[] = { border_brightness, border_brightness, border_brightness, 1.f};
            CHK(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor)); 
            CHK(glActiveTexture(GL_TEXTURE0));
        }
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva bind light depth textures");
    if (!lightmap_indices_depth.empty()) {
        for (size_t i : lightmap_indices) {
            std::string mname = "lightmap_depth." + filtered_lights.at(i).second->resource_suffix;
            const auto& light_vp = secondary_rendering_resources_->get_vp(mname);
            auto mvp_light = dot2d(light_vp, m.affine());
            CHK(glUniformMatrix4fv(rp.mvp_light_locations.at(i), 1, GL_TRUE, mvp_light.casted<float>().flat_begin()));

            CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_light(i))));
            CHK(glBindTexture(GL_TEXTURE_2D, secondary_rendering_resources_->get_texture({.color = mname, .color_mode = ColorMode::GRAYSCALE})));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
            CHK(glActiveTexture(GL_TEXTURE0));
        }
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva bind normalmap texture");
    if (tic.ntextures_normal != 0) {
        size_t i = 0;
        for (const auto& t : cva->material.textures) {
            if (!t.texture_descriptor.normal.empty()) {
                CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_normal(i))));
                CHK(glBindTexture(GL_TEXTURE_2D, rcva_->rendering_resources_->get_normalmap_texture(t.texture_descriptor)));
                setup_texture();
                CHK(glActiveTexture(GL_TEXTURE0));
            }
            ++i;
        }
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva bind reflection texture");
    if (tic.ntextures_reflection != 0) {
        CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_reflection())));
        CHK(glBindTexture(GL_TEXTURE_CUBE_MAP, rcva_->rendering_resources_->get_cubemap(reflection_map)));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        CHK(glActiveTexture(GL_TEXTURE0));
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva bind dirtmap texture");
    if (tic.ntextures_dirt != 0) {
        std::string mname = "dirtmap";
        {
            const auto& dirtmap_vp = secondary_rendering_resources_->get_vp(mname);
            auto mvp_dirtmap = dot2d(dirtmap_vp, m.affine());
            CHK(glUniformMatrix4fv(rp.mvp_dirtmap_location, 1, GL_TRUE, mvp_dirtmap.casted<float>().flat_begin()));
        }

        CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_dirt(0))));
        CHK(glBindTexture(GL_TEXTURE_2D, secondary_rendering_resources_->get_texture({.color = mname, .color_mode = ColorMode::GRAYSCALE})));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        {
            GLint p = get_wrap_param(secondary_rendering_resources_->get_texture_wrap(mname));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, p));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, p));
        }
        CHK(glActiveTexture(GL_TEXTURE0));

        CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_dirt(1))));
        CHK(glBindTexture(GL_TEXTURE_2D, rcva_->rendering_resources_->get_texture({.color = cva->material.dirt_texture})));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, get_wrap_param(cva->material.wrap_mode_s)));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, get_wrap_param(cva->material.wrap_mode_t)));
        CHK(glActiveTexture(GL_TEXTURE0));
    }
    if (tic.ntextures_interior != 0) {
        for (size_t i = 0; i < INTERIOR_COUNT; ++i) {
            CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_interior(i))));
            CHK(glBindTexture(GL_TEXTURE_2D, rcva_->rendering_resources_->get_texture({.color = cva->material.interior_textures[i], .color_mode = ColorMode::RGB})));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            CHK(glActiveTexture(GL_TEXTURE0));
        }
    }
    if (tic.ntextures_specular != 0) {
        assert_true(tic.ntextures_specular == 1);
        assert_true(cva->material.textures.size() == 1);
        assert_true(!cva->material.textures[0].texture_descriptor.specular.empty());
        CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + tic.id_specular())));
        CHK(glBindTexture(GL_TEXTURE_2D, rcva_->rendering_resources_->get_texture({.color = cva->material.textures[0].texture_descriptor.specular, .color_mode = ColorMode::RGB})));
        setup_texture();
        CHK(glActiveTexture(GL_TEXTURE0));
    }
    const SubstitutionInfo& si = rcva_->get_vertex_array(cva);
    if ((render_pass.external.pass != ExternalRenderPassType::DIRTMAP) &&
        !is_lightmap &&
        cva->material.draw_distance_noperations > 0 &&
        (
            std::isnan(render_config.draw_distance_add) ||
            (render_config.draw_distance_add != INFINITY)))
    {
        if (!rcva_->triangles_res_->bone_indices.empty()) {
            throw std::runtime_error("Draw distance incompatible with animations");
        }
        const_cast<SubstitutionInfo&>(si).delete_triangles_far_away(
            iv.t().casted<float>(),
            m.casted<float, float>(),
            std::isnan(render_config.draw_distance_add)
                ? cva->material.draw_distance_add
                : render_config.draw_distance_add,
            std::isnan(render_config.draw_distance_slop)
                ? cva->material.draw_distance_slop
                : render_config.draw_distance_slop,
            cva->material.draw_distance_noperations,
            true,  // run_in_background
            true); // is_static
    }
    LOG_INFO("RenderableColoredVertexArray::render_cva glBindVertexArray");
    {
        MaterialRenderConfigGuard mrcf{ cva->material };
        CHK(glBindVertexArray(si.va_.vertex_array));
        LOG_INFO("RenderableColoredVertexArray::render_cva glDrawArrays");
        if (has_instances) {
            CHK(glDrawArraysInstanced(GL_TRIANGLES, 0, (GLsizei)(3 * si.ntriangles_), (GLsizei)rcva_->instances_->at(si.cva_.get()).size()));
        } else {
            CHK(glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(3 * si.ntriangles_)));
        }
        CHK(glBindVertexArray(0));
    }
    // CHK(glFlush());
    LOG_INFO("RenderableColoredVertexArray::render_cva glDrawArrays finished");
}

void RenderableColoredVertexArray::render(
    const FixedArray<double, 4, 4>& mvp,
    const TransformationMatrix<float, double, 3>& m,
    const TransformationMatrix<float, double, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const RenderPass& render_pass,
    const AnimationState* animation_state,
    const ColorStyle* color_style) const
{
    LOG_FUNCTION("RenderableColoredVertexArray::render");
    if (render_pass.external.pass == ExternalRenderPassType::DIRTMAP) {
        return;
    }
    #ifdef DEBUG
    rcva_->triangles_res_->check_consistency();
    #endif
    std::vector<OffsetAndQuaternion<float, float>> absolute_bone_transformations =
        calculate_absolute_bone_transformations(animation_state);
    for (auto& cva : aggregate_off_) {
        // if (cva->name.find("street") != std::string::npos) {
        //     continue;
        // }
        // if (cva->name.find("path") != std::string::npos) {
        //     continue;
        // }
        render_cva(
            cva,
            absolute_bone_transformations,
            mvp,
            m,
            iv,
            lights,
            scene_graph_config,
            render_config,
            render_pass,
            animation_state,
            color_style);
    }
}

bool RenderableColoredVertexArray::requires_render_pass(ExternalRenderPassType render_pass) const {
    if (aggregate_off_.empty()) {
        return false;
    }
    if (any(render_pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK)) {
        return required_occluder_passes_.contains(render_pass);
    }
    return true;
}

bool RenderableColoredVertexArray::requires_blending_pass(ExternalRenderPassType render_pass) const {
    if (!requires_blending_pass_) {
        return false;
    }
    if (any(render_pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK)) {
        return required_occluder_passes_.contains(render_pass);
    }
    return true;
}

int RenderableColoredVertexArray::continuous_blending_z_order() const {
    if (continuous_blending_z_order_ == CONTINUOUS_BLENDING_Z_ORDER_CONFLICTING) {
        throw std::runtime_error("Conflicting z_orders");
    }
    return continuous_blending_z_order_;
}

void RenderableColoredVertexArray::append_sorted_aggregates_to_queue(
    const FixedArray<double, 4, 4>& mvp,
    const TransformationMatrix<float, double, 3>& m,
    const FixedArray<double, 3>& offset,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    std::list<std::pair<float, std::shared_ptr<ColoredVertexArray<float>>>>& aggregate_queue) const
{
    for (const auto& cva : aggregate_sorted_continuously_) {
        VisibilityCheck vc{mvp};
        if (vc.is_visible(cva->material, UINT32_MAX, scene_graph_config, external_render_pass.pass))
        {
            TransformationMatrix<float, double, 3> mo{m.R(), m.t() - offset};
            aggregate_queue.push_back({ vc.sorting_key(cva->material), std::move(cva->transformed<float>(mo, "_transformed_tm")) });
        }
    }
}

void RenderableColoredVertexArray::append_large_aggregates_to_queue(
    const TransformationMatrix<float, double, 3>& m,
    const FixedArray<double, 3>& offset,
    const SceneGraphConfig& scene_graph_config,
    std::list<std::shared_ptr<ColoredVertexArray<float>>>& aggregate_queue) const
{
    for (const auto& cva : aggregate_once_) {
        TransformationMatrix<float, double, 3> mo{m.R(), m.t() - offset};
        aggregate_queue.push_back(std::move(cva->transformed<float>(mo, "_transformed_tm")));
    }
}

void RenderableColoredVertexArray::append_sorted_instances_to_queue(
    const FixedArray<double, 4, 4>& mvp,
    const TransformationMatrix<float, double, 3>& m,
    const FixedArray<double, 3>& offset,
    uint32_t billboard_id,
    const SceneGraphConfig& scene_graph_config,
    SmallInstancesQueues& instances_queues) const
{
    instances_queues.insert(
        instances_sorted_continuously_,
        mvp,
        m,
        offset,
        billboard_id,
        scene_graph_config);
}

void RenderableColoredVertexArray::append_large_instances_to_queue(
    const FixedArray<double, 4, 4>& mvp,
    const TransformationMatrix<float, double, 3>& m,
    const FixedArray<double, 3>& offset,
    uint32_t billboard_id,
    const SceneGraphConfig& scene_graph_config,
    LargeInstancesQueue& instances_queue) const
{
    instances_queue.insert(
        instances_once_,
        mvp,
        m,
        offset,
        billboard_id,
        scene_graph_config,
        InvisibilityHandling::RAISE);
    if (any(instances_queue.render_pass() & ExternalRenderPassType::IS_STATIC_MASK)) {
        instances_queue.insert(
            instances_sorted_continuously_,
            mvp,
            m,
            offset,
            billboard_id,
            scene_graph_config,
            InvisibilityHandling::SKIP);
    }
}

AxisAlignedBoundingBox<float, 3> RenderableColoredVertexArray::aabb() const {
    AxisAlignedBoundingBox<float, 3> result;
    for (auto& cva : aggregate_off_) {
        for (const auto& v : cva->vertices()) {
            result.extend(v);
        }
    }
    return result;
}

void RenderableColoredVertexArray::print_stats(std::ostream& ostr) const {
    auto print_list = [&ostr]<typename TPos>(const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas, const std::string& name) {
        ostr << name << '\n';
        ostr << "#triangle lists: " << cvas.size() << '\n';
        size_t i = 0;
        for (auto& cva : cvas) {
            ostr << "triangle list " << i << " #lines: " << cva->lines.size() << '\n';
            ostr << "triangle list " << i << " #tris:  " << cva->triangles.size() << '\n';
        }
    };
    print_list(aggregate_off_, "aggregate_off");
    print_list(aggregate_once_, "aggregate_once");
    print_list(aggregate_sorted_continuously_, "aggregate_sorted_continuously");
    print_list(instances_once_, "instances_once");
    print_list(instances_sorted_continuously_, "instances_sorted_continuously");
}

std::ostream& Mlib::operator << (std::ostream& ostr, const RenderableColoredVertexArray& rcvi)
{
    rcvi.print_stats(ostr);
    return ostr;
}
