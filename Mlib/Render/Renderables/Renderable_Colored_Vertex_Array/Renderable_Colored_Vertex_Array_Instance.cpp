#include "Renderable_Colored_Vertex_Array_Instance.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Transformed_Colored_Vertex_Array.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Colored_Render_Program.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array.hpp>
#include <Mlib/Render/Renderables/Renderable_Colored_Vertex_Array/Substitution_Info.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Light.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Style.hpp>
#include <Mlib/Scene_Graph/Visibility_Check.hpp>

using namespace Mlib;

RenderableColoredVertexArrayInstance::RenderableColoredVertexArrayInstance(
    const std::shared_ptr<const RenderableColoredVertexArray>& rcva,
    const SceneNodeResourceFilter& resource_filter)
: rcva_{rcva}
{
#ifdef DEBUG
    rcva_->triangles_res_->check_consistency();
#endif
    requires_render_pass_ = false;
    requires_blending_pass_ = false;
    size_t i = 0;
    for (const auto& t : rcva->triangles_res_->cvas) {
        if (resource_filter.matches(i++, t->name)) {
            triangles_res_subset_.push_back(t);
            if (t->material.aggregate_mode == AggregateMode::OFF) {
                requires_render_pass_ = true;
            }
            if ((t->material.blend_mode == BlendMode::CONTINUOUS) &&
                (t->material.aggregate_mode == AggregateMode::OFF))
            {
                requires_blending_pass_ = true;
            }
        }
    }
}

RenderableColoredVertexArrayInstance::~RenderableColoredVertexArrayInstance()
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

std::vector<OffsetAndQuaternion<float>> RenderableColoredVertexArrayInstance::calculate_absolute_bone_transformations(const Style* style) const
{
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
        auto poses = rcva_->rendering_resources_.scene_node_resources().get_poses(
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

void RenderableColoredVertexArrayInstance::render_cva(
    const std::shared_ptr<ColoredVertexArray>& cva,
    const std::vector<OffsetAndQuaternion<float>>& absolute_bone_transformations,
    const FixedArray<float, 4, 4>& mvp,
    const TransformationMatrix<float>& m,
    const TransformationMatrix<float>& iv,
    const std::list<std::pair<TransformationMatrix<float>, Light*>>& lights,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const RenderPass& render_pass,
    const Style* style) const
{
    if (render_pass.internal == InternalRenderPass::INITIAL && cva->material.blend_mode == BlendMode::CONTINUOUS) {
        return;
    }
    if (render_pass.internal == InternalRenderPass::BLENDED && cva->material.blend_mode != BlendMode::CONTINUOUS) {
        return;
    }
    if (render_pass.external.pass == ExternalRenderPass::LIGHTMAP_TO_TEXTURE && render_pass.external.black_node_name.empty() && cva->material.occluder_type == OccluderType::OFF) {
        return;
    }
    VisibilityCheck vc{mvp};
    // Instance arrays are large and therefore do not need a visibility check.
    if (rcva_->instances_ == nullptr) {
        if (cva->material.aggregate_mode != AggregateMode::OFF) {
            return;
        }
        if (!vc.is_visible(cva->material, scene_graph_config, render_pass.external))
        {
            return;
        }
    }

    std::vector<std::pair<TransformationMatrix<float>, Light*>> filtered_lights;
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
    bool has_texture = !cva->material.texture_descriptor.color.empty() && ((render_pass.external.pass != ExternalRenderPass::LIGHTMAP_TO_TEXTURE) || (cva->material.blend_mode != BlendMode::OFF));
    bool has_lightmap_color = (cva->material.occluded_type == OccludedType::LIGHT_MAP_COLOR) && (render_pass.external.pass != ExternalRenderPass::LIGHTMAP_TO_TEXTURE) && (!cva->material.ambience.all_equal(0) || !cva->material.diffusivity.all_equal(0) || !cva->material.specularity.all_equal(0));
    bool has_lightmap_depth = (cva->material.occluded_type == OccludedType::LIGHT_MAP_DEPTH) && (render_pass.external.pass != ExternalRenderPass::LIGHTMAP_TO_TEXTURE) && (!cva->material.ambience.all_equal(0) || !cva->material.diffusivity.all_equal(0) || !cva->material.specularity.all_equal(0));
    bool has_normalmap = (!cva->material.texture_descriptor.normal.empty()) && (render_pass.external.pass != ExternalRenderPass::LIGHTMAP_TO_TEXTURE);
    bool has_dirtmap = (!cva->material.dirt_texture.empty()) && (render_pass.external.pass != ExternalRenderPass::LIGHTMAP_TO_TEXTURE);
    bool has_instances = (rcva_->instances_ != nullptr);
    bool has_lookat = (cva->material.transformation_mode == TransformationMode::POSITION_LOOKAT);
    if (!has_texture && has_dirtmap) {
        std::runtime_error("Combination of (!has_texture && has_dirtmap) is not supported. Texture: " + cva->material.texture_descriptor.color);
    }
    FixedArray<float, 3> ambience;
    FixedArray<float, 3> diffusivity;
    FixedArray<float, 3> specularity;
    if (!filtered_lights.empty() && (render_pass.external.pass != ExternalRenderPass::LIGHTMAP_TO_TEXTURE)) {
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
    if (!cva->material.texture_descriptor.color.empty()) {
        if (cva->material.texture_descriptor.color_mode == ColorMode::UNDEFINED) {
            throw std::runtime_error("Material's color texture \"" + cva->material.texture_descriptor.color + "\" has undefined color mode");
        }
        if ((cva->material.blend_mode == BlendMode::OFF) && (cva->material.texture_descriptor.color_mode == ColorMode::RGBA)) {
            throw std::runtime_error("Opaque material's color texture \"" + cva->material.texture_descriptor.color + "\" was loaded as RGBA");
        }
        if ((cva->material.blend_mode != BlendMode::OFF) && (cva->material.texture_descriptor.color_mode == ColorMode::RGB)) {
            throw std::runtime_error("Transparent material's color texture \"" + cva->material.texture_descriptor.color + "\" was not loaded as RGB");
        }
    }
    LOG_INFO("RenderableColoredVertexArrayInstance::render get_render_program");
    const ColoredRenderProgram& rp = rcva_->get_render_program(
        {
            .occluder_type = render_pass.external.black_node_name.empty() ? cva->material.occluder_type : OccluderType::BLACK,
            .nlights = filtered_lights.size(),
            .blend_mode = cva->material.blend_mode,
            .has_texture = has_texture,
            .has_lightmap_color = has_lightmap_color,
            .has_lightmap_depth = has_lightmap_depth,
            .has_normalmap = has_normalmap,
            .has_dirtmap = has_dirtmap,
            .has_instances = has_instances,
            .has_lookat = has_lookat,
            .reorient_normals = reorient_normals,
            .calculate_lightmap = render_pass.external.pass == ExternalRenderPass::LIGHTMAP_TO_TEXTURE,
            .ambience = OrderableFixedArray{ambience},
            .diffusivity = OrderableFixedArray{diffusivity},
            .specularity = OrderableFixedArray{specularity},
            .orthographic = vc.orthographic(),
            // Not using NAN for ordering.
            .dirtmap_discreteness = has_dirtmap ? rcva_->rendering_resources_.get_discreteness("dirtmap") : -1234},
        filtered_lights,
        light_noshadow_indices,
        light_shadow_indices,
        black_shadow_indices);
    LOG_INFO("RenderableColoredVertexArrayInstance::render glUseProgram");
    CHK(glUseProgram(rp.program));
    LOG_INFO("RenderableColoredVertexArrayInstance::render mvp");
    CHK(glUniformMatrix4fv(rp.mvp_location, 1, GL_TRUE, (const GLfloat*) mvp.flat_begin()));
    LOG_INFO("RenderableColoredVertexArrayInstance::render textures");
    if (has_texture) {
        CHK(glUniform1i(rp.texture1_location, 0));
    }
    assert_true(!(has_lightmap_color && has_lightmap_depth));
    if (has_lightmap_color) {
        size_t i = 0;
        for (const auto& l : filtered_lights) {
            if (l.second->shadow) {
                CHK(glUniform1i(rp.texture_lightmap_color_locations.at(i), 1 + i));
            }
            ++i;
        }
    }
    if (has_lightmap_depth) {
        for (size_t i = 0; i < filtered_lights.size(); ++i) {
            CHK(glUniform1i(rp.texture_lightmap_depth_locations.at(i), 1 + i));
        }
    }
    if (has_normalmap) {
        CHK(glUniform1i(rp.texture_normalmap_location, 1 + filtered_lights.size()));
    }
    if (has_dirtmap) {
        CHK(glUniform1i(rp.texture_dirtmap_location, 1 + has_normalmap + filtered_lights.size()));
        CHK(glUniform1i(rp.texture_dirt_location, 2 + has_normalmap + filtered_lights.size()));
    }
    LOG_INFO("RenderableColoredVertexArrayInstance::render lights");
    if (any(diffusivity != 0.f) || any(specularity != 0.f)) {
        CHK(glUniformMatrix4fv(rp.m_location, 1, GL_TRUE, (const GLfloat*) m.affine().flat_begin()));
        // CHK(glUniform3fv(rp.light_position_location, 1, (const GLfloat*) t3_from_4x4(filtered_lights.front().first).flat_begin()));
        size_t i = 0;
        for (const auto& l : filtered_lights) {
            CHK(glUniform3fv(rp.light_dir_locations.at(i++), 1, (const GLfloat*) z3_from_3x3(l.first.R()).flat_begin()));
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
    if (has_lookat || any(specularity != 0.f)) {
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
    LOG_INFO("RenderableColoredVertexArrayInstance::render bind texture");
    if (has_texture) {
        LOG_INFO("RenderableColoredVertexArrayInstance::render get texture \"" + cva->material.texture + '"');
        GLuint texture = rcva_->rendering_resources_.get_texture(cva->material.texture_descriptor);
        LOG_INFO("RenderableColoredVertexArrayInstance::render bind texture \"" + cva->material.texture + '"');
        CHK(glBindTexture(GL_TEXTURE_2D, texture));
        LOG_INFO("RenderableColoredVertexArrayInstance::render clamp texture \"" + cva->material.texture + '"');
        if (cva->material.wrap_mode_s == WrapMode::REPEAT) {
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        } else {
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        }
        if (cva->material.wrap_mode_t == WrapMode::REPEAT) {
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
        } else {
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        }
    }
    assert_true(!(has_lightmap_color && has_lightmap_depth));
    LOG_INFO("RenderableColoredVertexArrayInstance::render bind light color textures");
    if (has_lightmap_color) {
        size_t i = 0;
        for (const auto& l : filtered_lights) {
            if (l.second->shadow) {
                std::string mname = "lightmap_color" + l.second->node_name;
                const auto& light_vp = rcva_->rendering_resources_.get_vp(mname);
                auto mvp_light = dot2d(light_vp, m.affine());
                CHK(glUniformMatrix4fv(rp.mvp_light_locations.at(i), 1, GL_TRUE, (const GLfloat*) mvp_light.flat_begin()));
                
                CHK(glActiveTexture(GL_TEXTURE0 + 1 + i));
                CHK(glBindTexture(GL_TEXTURE_2D, rcva_->rendering_resources_.get_texture({color: mname, color_mode: ColorMode::RGB})));
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
                float borderColor[] = { 1.f, 1.f, 1.f, 1.f};
                CHK(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor)); 
                CHK(glActiveTexture(GL_TEXTURE0));
            }
            ++i;
        }
    }
    LOG_INFO("RenderableColoredVertexArrayInstance::render bind light depth textures");
    if (has_lightmap_depth) {
        size_t i = 0;
        for (const auto& l : filtered_lights) {
            if (l.second->shadow) {
                std::string mname = "lightmap_depth" + l.second->node_name;
                const auto& light_vp = rcva_->rendering_resources_.get_vp(mname);
                auto mvp_light = dot2d(light_vp, m.affine());
                CHK(glUniformMatrix4fv(rp.mvp_light_locations.at(i), 1, GL_TRUE, (const GLfloat*) mvp_light.flat_begin()));

                CHK(glActiveTexture(GL_TEXTURE0 + 1 + i));
                CHK(glBindTexture(GL_TEXTURE_2D, rcva_->rendering_resources_.get_texture({color: mname, color_mode: ColorMode::RGB})));
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
                CHK(glActiveTexture(GL_TEXTURE0));
            }
            ++i;
        }
    }
    LOG_INFO("RenderableColoredVertexArrayInstance::render bind normalmap texture");
    if (has_normalmap) {
        CHK(glActiveTexture(GL_TEXTURE0 + 1 + filtered_lights.size()));
        CHK(glBindTexture(GL_TEXTURE_2D, rcva_->rendering_resources_.get_texture({color: cva->material.texture_descriptor.normal, color_mode: ColorMode::RGB})));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, get_wrap_param(cva->material.wrap_mode_s)));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, get_wrap_param(cva->material.wrap_mode_t)));
        CHK(glActiveTexture(GL_TEXTURE0));
    }
    LOG_INFO("RenderableColoredVertexArrayInstance::render bind dirtmap texture");
    if (has_dirtmap) {
        std::string mname = "dirtmap";
        const auto& dirtmap_vp = rcva_->rendering_resources_.get_vp(mname);
        auto mvp_dirtmap = dot2d(dirtmap_vp, m.affine());
        CHK(glUniformMatrix4fv(rp.mvp_dirtmap_location, 1, GL_TRUE, (const GLfloat*) mvp_dirtmap.flat_begin()));

        CHK(glActiveTexture(GL_TEXTURE0 + 1 + has_normalmap + filtered_lights.size()));
        CHK(glBindTexture(GL_TEXTURE_2D, rcva_->rendering_resources_.get_texture({color: mname, color_mode: ColorMode::RGB})));
        {
            GLint p = get_wrap_param(rcva_->rendering_resources_.get_texture_wrap(mname));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, p));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, p));
        }
        CHK(glActiveTexture(GL_TEXTURE0));

        CHK(glActiveTexture(GL_TEXTURE0 + 2 + has_normalmap + filtered_lights.size()));
        CHK(glBindTexture(GL_TEXTURE_2D, rcva_->rendering_resources_.get_texture({color: cva->material.dirt_texture, color_mode: ColorMode::RGB})));
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
            break;
        default:
            throw std::runtime_error("Unknown blend_mode");
    }
    const SubstitutionInfo& si = rcva_->get_vertex_array(cva);
    if ((render_pass.external.pass != ExternalRenderPass::DIRTMAP) &&
        (render_pass.external.pass != ExternalRenderPass::LIGHTMAP_TO_TEXTURE) &&
        cva->material.draw_distance_noperations > 0)
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
            true);
    }
    LOG_INFO("RenderableColoredVertexArrayInstance::render glBindVertexArray");
    CHK(glBindVertexArray(si.va.vertex_array));
    LOG_INFO("RenderableColoredVertexArrayInstance::render glDrawArrays");
    if (has_instances) {
        CHK(glDrawArraysInstanced(GL_TRIANGLES, 0, 3 * si.ntriangles, rcva_->instances_->at(si.cva.get()).size()));
    } else {
        CHK(glDrawArrays(GL_TRIANGLES, 0, 3 * si.ntriangles));
    }
    CHK(glBindVertexArray(0));
    CHK(glDisable(GL_CULL_FACE));
    CHK(glDisable(GL_BLEND));
    CHK(glBlendFunc(GL_ONE, GL_ZERO));
    CHK(glDepthMask(GL_TRUE));
    LOG_INFO("RenderableColoredVertexArrayInstance::render glDrawArrays finished");
}

void RenderableColoredVertexArrayInstance::render(
    const FixedArray<float, 4, 4>& mvp,
    const TransformationMatrix<float>& m,
    const TransformationMatrix<float>& iv,
    const std::list<std::pair<TransformationMatrix<float>, Light*>>& lights,
    const SceneGraphConfig& scene_graph_config,
    const RenderConfig& render_config,
    const RenderPass& render_pass,
    const Style* style) const
{
    LOG_FUNCTION("RenderableColoredVertexArrayInstance::render");
    if (render_pass.external.pass == ExternalRenderPass::DIRTMAP) {
        return;
    }
    #ifdef DEBUG
    rcva_->triangles_res_->check_consistency();
    #endif
    std::vector<OffsetAndQuaternion<float>> absolute_bone_transformations =
        calculate_absolute_bone_transformations(style);
    for (auto& cva : triangles_res_subset_) {
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

bool RenderableColoredVertexArrayInstance::requires_render_pass() const {
    return requires_render_pass_;
}

bool RenderableColoredVertexArrayInstance::requires_blending_pass() const {
    return requires_blending_pass_;
}

void RenderableColoredVertexArrayInstance::append_sorted_aggregates_to_queue(
    const FixedArray<float, 4, 4>& mvp,
    const TransformationMatrix<float>& m,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    std::list<std::pair<float, std::shared_ptr<ColoredVertexArray>>>& aggregate_queue) const
{
    for (const auto& cva : triangles_res_subset_) {
        if (cva->material.aggregate_mode == AggregateMode::SORTED_CONTINUOUSLY) {
            if (VisibilityCheck{mvp}.is_visible(cva->material, scene_graph_config, external_render_pass))
            {
                float sorting_key = (cva->material.blend_mode == BlendMode::CONTINUOUS)
                    ? -mvp(2, 3)
                    : -INFINITY;
                aggregate_queue.push_back(std::make_pair(sorting_key, std::move(cva->transformed(m))));
            }
        }
    }
}

void RenderableColoredVertexArrayInstance::append_large_aggregates_to_queue(
    const TransformationMatrix<float>& m,
    const SceneGraphConfig& scene_graph_config,
    std::list<std::shared_ptr<ColoredVertexArray>>& aggregate_queue) const
{
    for (const auto& cva : triangles_res_subset_) {
        if (cva->material.aggregate_mode == AggregateMode::ONCE) {
            aggregate_queue.push_back(std::move(cva->transformed(m)));
        }
    }
}

void RenderableColoredVertexArrayInstance::append_sorted_instances_to_queue(
    const FixedArray<float, 4, 4>& mvp,
    const TransformationMatrix<float>& m,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    std::list<std::pair<float, TransformedColoredVertexArray>>& instances_queue) const
{
    for (const auto& cva : triangles_res_subset_) {
        if (cva->material.aggregate_mode == AggregateMode::INSTANCES_SORTED_CONTINUOUSLY) {
            if (VisibilityCheck{mvp}.is_visible(cva->material, scene_graph_config, external_render_pass))
            {
                float sorting_key = (cva->material.blend_mode == BlendMode::CONTINUOUS)
                    ? -mvp(2, 3)
                    : -INFINITY;
                instances_queue.push_back(std::make_pair(sorting_key, TransformedColoredVertexArray{cva: cva, transformation_matrix: m}));
            }
        }
    }
}

void RenderableColoredVertexArrayInstance::append_large_instances_to_queue(
    const TransformationMatrix<float>& m,
    const SceneGraphConfig& scene_graph_config,
    std::list<TransformedColoredVertexArray>& aggregate_queue) const
{
    for (const auto& cva : triangles_res_subset_) {
        if (cva->material.aggregate_mode == AggregateMode::INSTANCES_ONCE) {
            aggregate_queue.push_back({cva: cva, transformation_matrix: m});
        }
    }
}

void RenderableColoredVertexArrayInstance::print_stats() const {
    std::cerr << "#triangle lists: " << triangles_res_subset_.size() << std::endl;
    size_t i = 0;
    for (auto& cva : triangles_res_subset_) {
        std::cerr << "triangle list " << i << " #lines: " << cva->lines.size() << std::endl;
        std::cerr << "triangle list " << i << " #tris:  " << cva->triangles.size() << std::endl;
    }
}
