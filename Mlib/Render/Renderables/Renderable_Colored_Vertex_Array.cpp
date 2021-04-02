#include "Renderable_Colored_Vertex_Array.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
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
#include <Mlib/Scene_Graph/Light.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Style.hpp>
#include <Mlib/Scene_Graph/Visibility_Check.hpp>
#include <Mlib/Strings/String.hpp>
#include <climits>

// #undef LOG_FUNCTION
// #undef LOG_INFO
// #define LOG_FUNCTION(msg) ::Mlib::Log log(msg)
// #define LOG_INFO(msg) log.info(msg)

using namespace Mlib;

RenderableColoredVertexArray::RenderableColoredVertexArray(
    const std::shared_ptr<const ColoredVertexArrayResource>& rcva,
    const SceneNodeResourceFilter& resource_filter)
: rcva_{rcva},
  continuous_blending_z_order_{INT_MAX},
  secondary_rendering_resources_{RenderingContextStack::rendering_resources()}
{
#ifdef DEBUG
    rcva_->triangles_res_->check_consistency();
#endif
    requires_render_pass_ = false;
    requires_blending_pass_ = false;
    size_t i = 0;
    for (const auto& t : rcva->triangles_res_->cvas) {
        if (resource_filter.matches(i++, t->name)) {
            if ((t->material.aggregate_mode == AggregateMode::OFF) ||
                (rcva->instances_ != nullptr))
            {
                rendered_triangles_res_subset_.push_back(t);
                requires_render_pass_ = true;
            } else {
                aggregate_triangles_res_subset_.push_back(t);
            }
            if ((t->material.blend_mode == BlendMode::CONTINUOUS) &&
                (t->material.aggregate_mode == AggregateMode::OFF))
            {
                requires_blending_pass_ = true;
                if (continuous_blending_z_order_ == INT_MAX) {
                    continuous_blending_z_order_ = t->material.continuous_blending_z_order;
                } else if (continuous_blending_z_order_ != t->material.continuous_blending_z_order) {
                    throw std::runtime_error("Conflicting z_orders");
                }
            }
        }
    }
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

std::vector<OffsetAndQuaternion<float>> RenderableColoredVertexArray::calculate_absolute_bone_transformations(const Style* style) const
{
    TIME_GUARD_DECLARE(time_guard, "calculate_absolute_bone_transformations", "calculate_absolute_bone_transformations");
    if (!rcva_->triangles_res_->bone_indices.empty()) {
        if (style == nullptr) {
            throw std::runtime_error("Animation without style");
        }
        if (style->animation_frame.name.empty()) {
            throw std::runtime_error("Animation frame has no name");
        }
        if (std::isnan(style->animation_frame.loop_time)) {
            throw std::runtime_error("Loop time is NAN");
        }
        auto poses = rcva_->rendering_resources_->scene_node_resources().get_poses(
            style->animation_frame.name,
            style->animation_frame.loop_time);
        std::vector<OffsetAndQuaternion<float>> ms = rcva_->triangles_res_->vectorize_joint_poses(poses);
        std::vector<OffsetAndQuaternion<float>> absolute_bone_transformations = rcva_->triangles_res_->skeleton->absolutify(ms);
        if (absolute_bone_transformations.size() != rcva_->triangles_res_->bone_indices.size()) {
            throw std::runtime_error("Number of bone indices differs from number of quaternions");
        }
        return absolute_bone_transformations;
    } else {
        return {};
    }
}

void RenderableColoredVertexArray::render_cva(
    const std::shared_ptr<ColoredVertexArray>& cva,
    const std::vector<OffsetAndQuaternion<float>>& absolute_bone_transformations,
    const FixedArray<float, 4, 4>& mvp,
    const TransformationMatrix<float, 3>& m,
    const TransformationMatrix<float, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, 3>, Light*>>& lights,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const RenderPass& render_pass,
    const Style* style) const
{
    LOG_FUNCTION("render_cva");
    TIME_GUARD_DECLARE(time_guard, "render_cva", (cva->material.textures.size() > 0) ? cva->material.textures.front().texture_descriptor.color : cva->name);
    // This check passes because the arrays are filtered in the constructor.
    assert_true((cva->material.aggregate_mode == AggregateMode::OFF) || (rcva_->instances_ != nullptr));
    if (render_pass.internal == InternalRenderPass::INITIAL && cva->material.blend_mode == BlendMode::CONTINUOUS) {
        return;
    }
    if (render_pass.internal == InternalRenderPass::BLENDED && cva->material.blend_mode != BlendMode::CONTINUOUS) {
        return;
    }
    if (render_pass.external.pass == ExternalRenderPassType::LIGHTMAP_TO_TEXTURE && render_pass.external.black_node_name.empty() && cva->material.occluder_type == OccluderType::OFF) {
        return;
    }
    VisibilityCheck vc{mvp};
    // Instance arrays are large and therefore do not need a visibility check.
    if (rcva_->instances_ == nullptr) {
        if (!vc.is_visible(cva->material, scene_graph_config, render_pass.external))
        {
            return;
        }
    }

    std::vector<std::pair<TransformationMatrix<float, 3>, Light*>> filtered_lights;
    std::vector<size_t> light_noshadow_indices;
    std::vector<size_t> light_shadow_indices;
    std::vector<size_t> black_shadow_indices;
    filtered_lights.reserve(lights.size());
    light_noshadow_indices.reserve(lights.size());
    light_shadow_indices.reserve(lights.size());
    black_shadow_indices.reserve(lights.size());
    {
        size_t i = 0;
        for (const auto& l : lights) {
            if (!l.second->only_black || cva->material.occluded_by_black) {
                filtered_lights.push_back(l);
                if (!l.second->only_black) {
                    if (l.second->shadow) {
                        light_shadow_indices.push_back(i++);
                    } else {
                        light_noshadow_indices.push_back(i++);
                    }
                } else {
                    if (!l.second->shadow) {
                        throw std::runtime_error("Only-black light marked as not shadowed");
                    }
                    black_shadow_indices.push_back(i++);
                }
            }
        }
    }
    std::vector<BlendMapTexture*> blended_textures(cva->material.textures.size());
    for (size_t i = 0; i < blended_textures.size(); ++i) {
        blended_textures[i] = &cva->material.textures[i];
    }
    for (const auto& t : cva->material.textures) {
        if (t.texture_descriptor.color.empty()) {
            throw std::runtime_error("Empty colors not supported, cva: " + cva->name);
        }
        if (t.texture_descriptor.color_mode == ColorMode::UNDEFINED) {
            throw std::runtime_error("Material's color texture \"" + t.texture_descriptor.color + "\" has undefined color mode");
        }
        if ((cva->material.blend_mode == BlendMode::OFF) && (t.texture_descriptor.color_mode == ColorMode::RGBA)) {
            throw std::runtime_error("Opaque material's color texture \"" + t.texture_descriptor.color + "\" was loaded as RGBA");
        }
        if ((cva->material.blend_mode != BlendMode::OFF) && (t.texture_descriptor.color_mode == ColorMode::RGB)) {
            throw std::runtime_error("Transparent material's color texture \"" + t.texture_descriptor.color + "\" was not loaded as RGB");
        }
    }
    bool color_requires_normal = !cva->material.diffusivity.all_equal(0) || !cva->material.specularity.all_equal(0);
    size_t ntextures_color = ((render_pass.external.pass != ExternalRenderPassType::LIGHTMAP_TO_TEXTURE) || (cva->material.blend_mode != BlendMode::OFF)) ? cva->material.textures.size() : 0;
    bool has_lightmap_color = (cva->material.occluded_type == OccludedType::LIGHT_MAP_COLOR) && (render_pass.external.pass != ExternalRenderPassType::LIGHTMAP_TO_TEXTURE) && (!cva->material.ambience.all_equal(0) || !cva->material.diffusivity.all_equal(0) || !cva->material.specularity.all_equal(0));
    bool has_lightmap_depth = (cva->material.occluded_type == OccludedType::LIGHT_MAP_DEPTH) && (render_pass.external.pass != ExternalRenderPassType::LIGHTMAP_TO_TEXTURE) && (!cva->material.ambience.all_equal(0) || !cva->material.diffusivity.all_equal(0) || !cva->material.specularity.all_equal(0));
    size_t ntextures_normal = color_requires_normal && render_config.normalmaps && cva->material.has_normalmap() && (render_pass.external.pass != ExternalRenderPassType::LIGHTMAP_TO_TEXTURE) ? cva->material.textures.size() : 0;
    bool has_dirtmap = (!cva->material.dirt_texture.empty()) && (render_pass.external.pass != ExternalRenderPassType::LIGHTMAP_TO_TEXTURE);
    bool has_instances = (rcva_->instances_ != nullptr);
    bool has_lookat = (cva->material.transformation_mode == TransformationMode::POSITION_LOOKAT);
    OrderableFixedArray<float, 4> alpha_distances;
    bool fragments_depend_on_distance;
    if (render_pass.external.pass == ExternalRenderPassType::LIGHTMAP_TO_TEXTURE) {
        fragments_depend_on_distance = false;
        alpha_distances = default_distances;
    } else {
        fragments_depend_on_distance = cva->material.fragments_depend_on_distance();
        alpha_distances = cva->material.alpha_distances;
    }
    bool fragments_depend_on_normal = (render_pass.external.pass != ExternalRenderPassType::LIGHTMAP_TO_TEXTURE) && cva->material.fragments_depend_on_normal();
    if ((ntextures_color == 0) && has_dirtmap) {
        throw std::runtime_error(
            "Combination of ((ntextures_color == 0) && has_dirtmap) is not supported. Textures: " +
            join(" ", cva->material.textures, [](const auto& v) { return v.texture_descriptor.color; }));
    }
    FixedArray<float, 3> ambience;
    FixedArray<float, 3> diffusivity;
    FixedArray<float, 3> specularity;
    if (!filtered_lights.empty() && (render_pass.external.pass != ExternalRenderPassType::LIGHTMAP_TO_TEXTURE)) {
        ambience = style && !all(style->ambience == -1.f) ? style->ambience : cva->material.ambience;
        diffusivity = style && !all(style->diffusivity == -1.f) ? style->diffusivity : cva->material.diffusivity;
        specularity = style && !all(style->specularity == -1.f) ? style->specularity : cva->material.specularity;
    } else {
        ambience = fixed_zeros<float, 3>();
        diffusivity = fixed_zeros<float, 3>();
        specularity = fixed_zeros<float, 3>();
    }
    if (filtered_lights.size() == 1) {
        ambience *= (filtered_lights.front().second->ambience != 0.f).casted<float>();
        diffusivity *= (filtered_lights.front().second->diffusivity != 0.f).casted<float>();
        specularity *= (filtered_lights.front().second->specularity != 0.f).casted<float>();
    }
    bool reorient_normals = !cva->material.cull_faces && (any(diffusivity != 0.f) || any(specularity != 0.f));
    LOG_INFO("RenderableColoredVertexArray::render get_render_program");
    const ColoredRenderProgram& rp = rcva_->get_render_program(
        {
            .occluder_type = render_pass.external.black_node_name.empty() ? cva->material.occluder_type : OccluderType::BLACK,
            .nlights = filtered_lights.size(),
            .blend_mode = cva->material.blend_mode,
            .alpha_distances = alpha_distances,
            .ntextures_color = ntextures_color,
            .ntextures_normal = ntextures_normal,
            .has_lightmap_color = has_lightmap_color,
            .has_lightmap_depth = has_lightmap_depth,
            .has_dirtmap = has_dirtmap,
            .has_instances = has_instances,
            .has_lookat = has_lookat,
            .reorient_normals = reorient_normals,
            .calculate_lightmap = render_pass.external.pass == ExternalRenderPassType::LIGHTMAP_TO_TEXTURE,
            .ambience = OrderableFixedArray{ambience},
            .diffusivity = OrderableFixedArray{diffusivity},
            .specularity = OrderableFixedArray{specularity},
            .orthographic = vc.orthographic(),
            .fragments_depend_on_distance = fragments_depend_on_distance,
            .fragments_depend_on_normal = fragments_depend_on_normal,
            // Not using NAN for ordering.
            .dirtmap_offset = has_dirtmap ? secondary_rendering_resources_->get_offset("dirtmap") : -1234,
            .dirtmap_discreteness = has_dirtmap ? secondary_rendering_resources_->get_discreteness("dirtmap") : -1234},
        filtered_lights,
        light_noshadow_indices,
        light_shadow_indices,
        black_shadow_indices,
        blended_textures);
    LOG_INFO("RenderableColoredVertexArray::render glUseProgram");
    CHK(glUseProgram(rp.program));
    LOG_INFO("RenderableColoredVertexArray::render mvp");
    CHK(glUniformMatrix4fv(rp.mvp_location, 1, GL_TRUE, (const GLfloat*) mvp.flat_begin()));
    LOG_INFO("RenderableColoredVertexArray::render textures");
    for (size_t i = 0; i < ntextures_color; ++i) {
        CHK(glUniform1i(rp.texture_color_locations.at(i), (GLint)i));
    }
    assert_true(!(has_lightmap_color && has_lightmap_depth));
    if (has_lightmap_color) {
        size_t i = 0;
        for (const auto& l : filtered_lights) {
            if (l.second->shadow) {
                CHK(glUniform1i(rp.texture_lightmap_color_locations.at(i), (GLint)(ntextures_color + i)));
            }
            ++i;
        }
    }
    if (has_lightmap_depth) {
        for (size_t i = 0; i < filtered_lights.size(); ++i) {
            CHK(glUniform1i(rp.texture_lightmap_depth_locations.at(i), (GLint)(ntextures_color + i)));
        }
    }
    if (ntextures_normal != 0) {
        size_t i = 0;
        for (const auto& t : cva->material.textures) {
            if (!t.texture_descriptor.normal.empty()) {
                CHK(glUniform1i(rp.texture_normalmap_locations.at(i), (GLint)(ntextures_color + filtered_lights.size() + i)));
            }
            ++i;
        }
    }
    if (has_dirtmap) {
        CHK(glUniform1i(rp.texture_dirtmap_location, (GLint)(ntextures_color + filtered_lights.size() + ntextures_normal + 0)));
        CHK(glUniform1i(rp.texture_dirt_location, (GLint)(ntextures_color + filtered_lights.size() + ntextures_normal + 1)));
    }
    LOG_INFO("RenderableColoredVertexArray::render lights");
    {
        bool light_dir_required = (any(diffusivity != 0.f) || any(specularity != 0.f));
        if (light_dir_required || fragments_depend_on_distance || fragments_depend_on_normal) {
            CHK(glUniformMatrix4fv(rp.m_location, 1, GL_TRUE, (const GLfloat*)m.affine().flat_begin()));
            // CHK(glUniform3fv(rp.light_position_location, 1, (const GLfloat*) t3_from_4x4(filtered_lights.front().first).flat_begin()));
            if (light_dir_required) {
                size_t i = 0;
                for (const auto& l : filtered_lights) {
                    CHK(glUniform3fv(rp.light_dir_locations.at(i++), 1, (const GLfloat*)z3_from_3x3(l.first.R()).flat_begin()));
                }
            }
        }
    }
    {
        size_t i = 0;
        for (const auto& l : filtered_lights) {
            if (any(ambience != 0.f)) {
                CHK(glUniform3fv(rp.light_ambiences.at(i), 1, (const GLfloat*) l.second->ambience.flat_begin()));
            }
            if (any(diffusivity != 0.f)) {
                CHK(glUniform3fv(rp.light_diffusivities.at(i), 1, (const GLfloat*) l.second->diffusivity.flat_begin()));
            }
            if (any(specularity != 0.f)) {
                CHK(glUniform3fv(rp.light_specularities.at(i), 1, (const GLfloat*) l.second->specularity.flat_begin()));
            }
            ++i;
        }
    }
    if (has_lookat || any(specularity != 0.f) || fragments_depend_on_distance) {
        if (vc.orthographic()) {
            auto d = z3_from_3x3(iv.R());
            d /= std::sqrt(sum(squared(d)));
            CHK(glUniform3fv(rp.view_dir, 1, (const GLfloat*) d.flat_begin()));
        } else {
            CHK(glUniform3fv(rp.view_pos, 1, (const GLfloat*) iv.t().flat_begin()));
        }
    }
    if (!rcva_->triangles_res_->bone_indices.empty()) {
        size_t i = 0;
        for (const auto& l : absolute_bone_transformations) {
            CHK(glUniform3fv(rp.pose_positions.at(i), 1, (const GLfloat*) l.offset().flat_begin()));
            CHK(glUniform4fv(rp.pose_quaternions.at(i), 1, (const GLfloat*) l.quaternion().vector().flat_begin()));
            ++i;
        }
    }
    LOG_INFO("RenderableColoredVertexArray::render bind texture");
    auto setup_texture = [&cva]() {
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, get_wrap_param(cva->material.wrap_mode_s)));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, get_wrap_param(cva->material.wrap_mode_t)));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    };
    if (ntextures_color != 0) {
        size_t i = 0;
        for (const auto& t : cva->material.textures) {
            LOG_INFO("RenderableColoredVertexArray::render get texture \"" + cva->material.texture_descriptor.color + '"');
            GLuint texture = rcva_->rendering_resources_->get_texture(t.texture_descriptor);
            LOG_INFO("RenderableColoredVertexArray::render bind texture \"" + cva->material.texture_descriptor.color + '"');
            CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + i)));
            CHK(glBindTexture(GL_TEXTURE_2D, texture));
            LOG_INFO("RenderableColoredVertexArray::render clamp texture \"" + cva->material.texture_descriptor.color + '"');
            setup_texture();
            CHK(glActiveTexture(GL_TEXTURE0));
            ++i;
        }
    }
    assert_true(!(has_lightmap_color && has_lightmap_depth));
    LOG_INFO("RenderableColoredVertexArray::render bind light color textures");
    if (has_lightmap_color) {
        size_t i = 0;
        for (const auto& l : filtered_lights) {
            if (l.second->shadow) {
                std::string mname = "lightmap_color." + l.second->node_name;
                const auto& light_vp = secondary_rendering_resources_->get_vp(mname);
                auto mvp_light = dot2d(light_vp, m.affine());
                CHK(glUniformMatrix4fv(rp.mvp_light_locations.at(i), 1, GL_TRUE, (const GLfloat*) mvp_light.flat_begin()));
                
                CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + ntextures_color + i)));
                CHK(glBindTexture(GL_TEXTURE_2D, secondary_rendering_resources_->get_texture({.color = mname, .color_mode = ColorMode::RGB})));
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
                float borderColor[] = { 1.f, 1.f, 1.f, 1.f};
                CHK(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor)); 
                CHK(glActiveTexture(GL_TEXTURE0));
            }
            ++i;
        }
    }
    LOG_INFO("RenderableColoredVertexArray::render bind light depth textures");
    if (has_lightmap_depth) {
        size_t i = 0;
        for (const auto& l : filtered_lights) {
            if (l.second->shadow) {
                std::string mname = "lightmap_depth" + l.second->node_name;
                const auto& light_vp = secondary_rendering_resources_->get_vp(mname);
                auto mvp_light = dot2d(light_vp, m.affine());
                CHK(glUniformMatrix4fv(rp.mvp_light_locations.at(i), 1, GL_TRUE, (const GLfloat*) mvp_light.flat_begin()));

                CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + ntextures_color + i)));
                CHK(glBindTexture(GL_TEXTURE_2D, secondary_rendering_resources_->get_texture({.color = mname, .color_mode = ColorMode::RGB})));
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
                CHK(glActiveTexture(GL_TEXTURE0));
            }
            ++i;
        }
    }
    LOG_INFO("RenderableColoredVertexArray::render bind normalmap texture");
    if (ntextures_normal != 0) {
        size_t i = 0;
        for (const auto& t : cva->material.textures) {
            if (!t.texture_descriptor.normal.empty()) {
                CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + ntextures_color + filtered_lights.size() + i)));
                CHK(glBindTexture(GL_TEXTURE_2D, rcva_->rendering_resources_->get_normalmap_texture(t.texture_descriptor)));
                setup_texture();
                CHK(glActiveTexture(GL_TEXTURE0));
            }
            ++i;
        }
    }
    LOG_INFO("RenderableColoredVertexArray::render bind dirtmap texture");
    if (has_dirtmap) {
        std::string mname = "dirtmap";
        {
            const auto& dirtmap_vp = secondary_rendering_resources_->get_vp(mname);
            auto mvp_dirtmap = dot2d(dirtmap_vp, m.affine());
            CHK(glUniformMatrix4fv(rp.mvp_dirtmap_location, 1, GL_TRUE, (const GLfloat*)mvp_dirtmap.flat_begin()));
        }

        CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + ntextures_color + filtered_lights.size() + ntextures_normal + 0)));
        CHK(glBindTexture(GL_TEXTURE_2D, secondary_rendering_resources_->get_texture({.color = mname, .color_mode = ColorMode::RGB})));
        {
            GLint p = get_wrap_param(secondary_rendering_resources_->get_texture_wrap(mname));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, p));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, p));
        }
        CHK(glActiveTexture(GL_TEXTURE0));

        CHK(glActiveTexture((GLenum)(GL_TEXTURE0 + ntextures_color + filtered_lights.size() + ntextures_normal + 1)));
        CHK(glBindTexture(GL_TEXTURE_2D, rcva_->rendering_resources_->get_texture({.color = cva->material.dirt_texture, .color_mode = ColorMode::RGB})));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, get_wrap_param(cva->material.wrap_mode_s)));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, get_wrap_param(cva->material.wrap_mode_t)));
        CHK(glActiveTexture(GL_TEXTURE0));
    }
    if (render_config.cull_faces && cva->material.cull_faces) {
        CHK(glEnable(GL_CULL_FACE));
    }
    switch(cva->material.blend_mode) {
        case BlendMode::OFF:
        case BlendMode::BINARY:
            break;
        case BlendMode::BINARY_ADD:
            CHK(glEnable(GL_BLEND));
            CHK(glBlendFunc(GL_ONE, GL_ONE));
            CHK(glDepthMask(GL_FALSE));
            break;
        case BlendMode::CONTINUOUS:
            CHK(glEnable(GL_BLEND));
            CHK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            CHK(glDepthMask(GL_FALSE));
            if (cva->material.depth_func_equal) {
                CHK(glDepthFunc(GL_EQUAL));
            }
            break;
        default:
            throw std::runtime_error("Unknown blend_mode");
    }
    const SubstitutionInfo& si = rcva_->get_vertex_array(cva);
    if ((render_pass.external.pass != ExternalRenderPassType::DIRTMAP) &&
        (render_pass.external.pass != ExternalRenderPassType::LIGHTMAP_TO_TEXTURE) &&
        cva->material.draw_distance_noperations > 0 &&
        (
            std::isnan(render_config.draw_distance_add) ||
            (render_config.draw_distance_add != INFINITY)))
    {
        if (!rcva_->triangles_res_->bone_indices.empty()) {
            throw std::runtime_error("Draw distance incompatible with animations");
        }
        const_cast<SubstitutionInfo&>(si).delete_triangles_far_away(
            iv.t(),
            m,
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
    LOG_INFO("RenderableColoredVertexArray::render glBindVertexArray");
    CHK(glBindVertexArray(si.va_.vertex_array));
    LOG_INFO("RenderableColoredVertexArray::render glDrawArrays");
    if (has_instances) {
        CHK(glDrawArraysInstanced(GL_TRIANGLES, 0, (GLsizei)(3 * si.ntriangles_), (GLsizei)rcva_->instances_->at(si.cva_.get()).size()));
    } else {
        CHK(glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(3 * si.ntriangles_)));
    }
    CHK(glBindVertexArray(0));
    CHK(glDisable(GL_CULL_FACE));
    CHK(glDisable(GL_BLEND));
    CHK(glBlendFunc(GL_ONE, GL_ZERO));
    CHK(glDepthMask(GL_TRUE));
    CHK(glDepthFunc(GL_LESS));
    // CHK(glFlush());
    LOG_INFO("RenderableColoredVertexArray::render glDrawArrays finished");
}

void RenderableColoredVertexArray::render(
    const FixedArray<float, 4, 4>& mvp,
    const TransformationMatrix<float, 3>& m,
    const TransformationMatrix<float, 3>& iv,
    const std::list<std::pair<TransformationMatrix<float, 3>, Light*>>& lights,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const RenderPass& render_pass,
    const Style* style) const
{
    LOG_FUNCTION("RenderableColoredVertexArray::render");
    if (render_pass.external.pass == ExternalRenderPassType::DIRTMAP) {
        return;
    }
    #ifdef DEBUG
    rcva_->triangles_res_->check_consistency();
    #endif
    std::vector<OffsetAndQuaternion<float>> absolute_bone_transformations =
        calculate_absolute_bone_transformations(style);
    for (auto& cva : rendered_triangles_res_subset_) {
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
            style);
    }
}

bool RenderableColoredVertexArray::requires_render_pass() const {
    return requires_render_pass_;
}

bool RenderableColoredVertexArray::requires_blending_pass() const {
    return requires_blending_pass_;
}

int RenderableColoredVertexArray::continuous_blending_z_order() const {
    return continuous_blending_z_order_;
}

void RenderableColoredVertexArray::append_sorted_aggregates_to_queue(
    const FixedArray<float, 4, 4>& mvp,
    const TransformationMatrix<float, 3>& m,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    std::list<std::pair<float, std::shared_ptr<ColoredVertexArray>>>& aggregate_queue) const
{
    for (const auto& cva : aggregate_triangles_res_subset_) {
        if (cva->material.aggregate_mode == AggregateMode::SORTED_CONTINUOUSLY) {
            if (VisibilityCheck{mvp}.is_visible(cva->material, scene_graph_config, external_render_pass))
            {
                float sorting_key = (cva->material.blend_mode == BlendMode::CONTINUOUS)
                    ? -mvp(2, 3)
                    : -INFINITY;
                aggregate_queue.push_back({ sorting_key, std::move(cva->transformed(m)) });
            }
        }
    }
}

void RenderableColoredVertexArray::append_large_aggregates_to_queue(
    const TransformationMatrix<float, 3>& m,
    const SceneGraphConfig& scene_graph_config,
    std::list<std::shared_ptr<ColoredVertexArray>>& aggregate_queue) const
{
    for (const auto& cva : aggregate_triangles_res_subset_) {
        if (cva->material.aggregate_mode == AggregateMode::ONCE) {
            aggregate_queue.push_back(std::move(cva->transformed(m)));
        }
    }
}

void RenderableColoredVertexArray::append_sorted_instances_to_queue(
    const FixedArray<float, 4, 4>& mvp,
    const TransformationMatrix<float, 3>& m,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    std::list<std::pair<float, TransformedColoredVertexArray>>& instances_queue) const
{
    for (const auto& cva : aggregate_triangles_res_subset_) {
        if (cva->material.aggregate_mode == AggregateMode::INSTANCES_SORTED_CONTINUOUSLY) {
            VisibilityCheck vc{ mvp };
            if (vc.is_visible(cva->material, scene_graph_config, external_render_pass))
            {
                float sorting_key = vc.sorting_key(cva->material, scene_graph_config);
                instances_queue.push_back({ sorting_key, TransformedColoredVertexArray{.cva = cva, .transformation_matrix = m} });
            }
        }
    }
}

void RenderableColoredVertexArray::append_large_instances_to_queue(
    const TransformationMatrix<float, 3>& m,
    const SceneGraphConfig& scene_graph_config,
    std::list<TransformedColoredVertexArray>& aggregate_queue) const
{
    for (const auto& cva : aggregate_triangles_res_subset_) {
        if (cva->material.aggregate_mode == AggregateMode::INSTANCES_ONCE) {
            aggregate_queue.push_back({.cva = cva, .transformation_matrix = m});
        }
    }
}

void RenderableColoredVertexArray::print_stats(std::ostream& ostr) const {
    auto print_list = [&ostr](const std::list<std::shared_ptr<ColoredVertexArray>>& cvas, const std::string& name) {
        ostr << name << '\n';
        ostr << "#triangle lists: " << cvas.size() << '\n';
        size_t i = 0;
        for (auto& cva : cvas) {
            ostr << "triangle list " << i << " #lines: " << cva->lines.size() << '\n';
            ostr << "triangle list " << i << " #tris:  " << cva->triangles.size() << '\n';
        }
    };
    print_list(rendered_triangles_res_subset_, "rendered");
    print_list(aggregate_triangles_res_subset_, "aggregate");
}

std::ostream& Mlib::operator << (std::ostream& ostr, const RenderableColoredVertexArray& rcvi)
{
    rcvi.print_stats(ostr);
    return ostr;
}
