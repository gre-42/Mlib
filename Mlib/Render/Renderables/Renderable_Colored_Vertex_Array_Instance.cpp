#include "Renderable_Colored_Vertex_Array.hpp"
#include "Renderable_Colored_Vertex_Array_Instance.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Render_Config.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Light.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>

using namespace Mlib;

RenderableColoredVertexArrayInstance::RenderableColoredVertexArrayInstance(
    const std::shared_ptr<RenderableColoredVertexArray>& rcva,
    const SceneNodeResourceFilter& resource_filter)
: rcva_{rcva}
{
    size_t i = 0;
    for(const auto& t : rcva->triangles_res_) {
        if (resource_filter.matches(i++, t->name)) {
            triangles_res_subset_.push_back(t);
        }
    }
}

void RenderableColoredVertexArrayInstance::render(const FixedArray<float, 4, 4>& mvp, const FixedArray<float, 4, 4>& m, const FixedArray<float, 4, 4>& iv, const std::list<std::pair<FixedArray<float, 4, 4>, Light*>>& lights, const SceneGraphConfig& scene_graph_config, const RenderConfig& render_config, const RenderPass& render_pass) {
    if (render_pass.external.pass == ExternalRenderPass::DIRTMAP) {
        return;
    }
    for(auto& cva : triangles_res_subset_) {
        if (render_pass.internal == InternalRenderPass::INITIAL && cva->material.blend_mode == BlendMode::CONTINUOUS) {
            continue;
        }
        if (render_pass.internal == InternalRenderPass::BLENDED && cva->material.blend_mode != BlendMode::CONTINUOUS) {
            continue;
        }
        if (cva->material.aggregate_mode != AggregateMode::OFF) {
            continue;
        }
        if (render_pass.external.pass == ExternalRenderPass::LIGHTMAP_TO_TEXTURE && render_pass.external.black_node_name.empty() && cva->material.occluder_type == OccluderType::OFF) {
            continue;
        }
        if (cva->material.is_small &&
            ((mvp(2, 3) < scene_graph_config.min_distance_small) ||
             (sum(squared(t3_from_4x4(mvp))) < squared(scene_graph_config.min_distance_small)) ||
             (sum(squared(t3_from_4x4(mvp))) > squared(scene_graph_config.max_distance_small))))
        {
            continue;
        }

        std::list<std::pair<FixedArray<float, 4, 4>, Light*>> filtered_lights;
        for(const auto& l : lights) {
            if (!l.second->only_black || cva->material.occluded_by_black) {
                filtered_lights.push_back(l);
            }
        }
        bool has_texture = rcva_->render_textures_ && !cva->material.texture.empty();
        bool has_lightmap_color = rcva_->render_textures_ && (cva->material.occluded_type == OccludedType::LIGHT_MAP_COLOR) && (render_pass.external.pass != ExternalRenderPass::LIGHTMAP_TO_TEXTURE) && (cva->material.diffusivity.is_nonzero() || cva->material.specularity.is_nonzero());
        bool has_lightmap_depth = rcva_->render_textures_ && (cva->material.occluded_type == OccludedType::LIGHT_MAP_DEPTH) && (render_pass.external.pass != ExternalRenderPass::LIGHTMAP_TO_TEXTURE) && (cva->material.diffusivity.is_nonzero() || cva->material.specularity.is_nonzero());
        bool has_dirtmap = rcva_->render_textures_ && (!cva->material.dirt_texture.empty()) && (render_pass.external.pass != ExternalRenderPass::LIGHTMAP_TO_TEXTURE);
        if (!has_texture && has_dirtmap) {
            std::runtime_error("Combination of (!has_texture && has_dirtmap) is not supported. Texture: " + cva->material.texture);
        }
        FixedArray<float, 3> ambience = (render_pass.external.pass != ExternalRenderPass::LIGHTMAP_TO_TEXTURE) ? cva->material.ambience : fixed_zeros<float, 3>();
        FixedArray<float, 3> diffusivity = !filtered_lights.empty() && (render_pass.external.pass != ExternalRenderPass::LIGHTMAP_TO_TEXTURE) ? cva->material.diffusivity : fixed_zeros<float, 3>();
        FixedArray<float, 3> specularity = !filtered_lights.empty() && (render_pass.external.pass != ExternalRenderPass::LIGHTMAP_TO_TEXTURE) ? cva->material.specularity : fixed_zeros<float, 3>();
        bool reorient_normals = !cva->material.cull_faces && (any(diffusivity != 0.f) || any(specularity != 0.f));
        const ColoredRenderProgram& rp = rcva_->get_render_program(
            {
                aggregate_mode: cva->material.aggregate_mode,
                occluder_type: render_pass.external.black_node_name.empty() ? cva->material.occluder_type : OccluderType::BLACK,
                nlights: filtered_lights.size(),
                blend_mode: cva->material.blend_mode,
                has_texture: has_texture,
                has_lightmap_color: has_lightmap_color,
                has_lightmap_depth: has_lightmap_depth,
                has_dirtmap: has_dirtmap,
                reorient_normals: reorient_normals,
                calculate_lightmap: render_pass.external.pass == ExternalRenderPass::LIGHTMAP_TO_TEXTURE,
                ambience: OrderableFixedArray{ambience},
                diffusivity: OrderableFixedArray{diffusivity},
                specularity: OrderableFixedArray{specularity}},
            filtered_lights);
        const VertexArray& va = rcva_->get_vertex_array(cva.get());
        CHK(glUseProgram(rp.program));
        CHK(glUniformMatrix4fv(rp.mvp_location, 1, GL_TRUE, (const GLfloat*) mvp.flat_begin()));
        if (has_texture) {
            CHK(glUniform1i(rp.texture1_location, 0));
        }
        assert_true(!(has_lightmap_color && has_lightmap_depth));
        if (has_lightmap_color) {
            for(size_t i = 0; i < filtered_lights.size(); ++i) {
                CHK(glUniform1i(rp.texture_lightmap_color_locations.at(i), 1 + i));
            }
        }
        if (has_lightmap_depth) {
            for(size_t i = 0; i < filtered_lights.size(); ++i) {
                CHK(glUniform1i(rp.texture_lightmap_depth_locations.at(i), 1 + i));
            }
        }
        if (has_dirtmap) {
            CHK(glUniform1i(rp.texture_dirtmap_location, 1 + filtered_lights.size()));
            CHK(glUniform1i(rp.texture_dirt_location, 2 + filtered_lights.size()));
        }
        if (any(diffusivity != 0.f) || any(specularity != 0.f)) {
            CHK(glUniformMatrix4fv(rp.m_location, 1, GL_TRUE, (const GLfloat*) m.flat_begin()));
            // CHK(glUniform3fv(rp.light_position_location, 1, (const GLfloat*) t3_from_4x4(filtered_lights.front().first).flat_begin()));
            size_t i = 0;
            for(const auto& l : filtered_lights) {
                CHK(glUniform3fv(rp.light_dir_locations.at(i), 1, (const GLfloat*) z3_from_4x4(l.first).flat_begin()));
                CHK(glUniform3fv(rp.light_colors.at(i), 1, (const GLfloat*) FixedArray<float, 3>{1, 1, 1}.flat_begin()));
                ++i;
            }
        }
        if (any(specularity != 0.f)) {
            CHK(glUniform3fv(rp.view_pos, 1, (const GLfloat*) t3_from_4x4(iv).flat_begin()));
        }
        CHK(glBindVertexArray(va.vertex_array));
        if (has_texture) {
            GLuint texture = rcva_->rendering_resources_->get_texture(
                cva->material.texture,
                cva->material.blend_mode != BlendMode::OFF,
                cva->material.mixed_texture,
                cva->material.overlap_npixels);
            CHK(glBindTexture(GL_TEXTURE_2D, texture));
            if (cva->material.clamp_mode_s == ClampMode::REPEAT) {
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
            } else {
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            }
            if (cva->material.clamp_mode_t == ClampMode::REPEAT) {
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
            } else {
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            }
        }
        assert_true(!(has_lightmap_color && has_lightmap_depth));
        if (has_lightmap_color) {
            size_t i = 0;
            for(const auto& l : filtered_lights) {
                std::string mname = "lightmap_color" + std::to_string(l.second->resource_index);
                const auto& light_vp = rcva_->rendering_resources_->get_vp(mname);
                auto mvp_light = dot2d(light_vp, m);
                CHK(glUniformMatrix4fv(rp.mvp_light_locations.at(i), 1, GL_TRUE, (const GLfloat*) mvp_light.flat_begin()));
                
                CHK(glActiveTexture(GL_TEXTURE0 + 1 + i));
                CHK(glBindTexture(GL_TEXTURE_2D, rcva_->rendering_resources_->get_texture(mname, false)));  // false=rgba
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
                float borderColor[] = { 1.f, 1.f, 1.f, 1.f};
                CHK(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor)); 
                CHK(glActiveTexture(GL_TEXTURE0));
                ++i;
            }
        }
        if (has_lightmap_depth) {
            size_t i = 0;
            for(const auto& l : filtered_lights) {
                std::string mname = "lightmap_depth" + std::to_string(l.second->resource_index);
                const auto& light_vp = rcva_->rendering_resources_->get_vp(mname);
                auto mvp_light = dot2d(light_vp, m);
                CHK(glUniformMatrix4fv(rp.mvp_light_locations.at(i), 1, GL_TRUE, (const GLfloat*) mvp_light.flat_begin()));

                CHK(glActiveTexture(GL_TEXTURE0 + 1 + i));
                CHK(glBindTexture(GL_TEXTURE_2D, rcva_->rendering_resources_->get_texture(mname, false)));  // false=rgba
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
                CHK(glActiveTexture(GL_TEXTURE0));
                ++i;
            }
        }
        if (has_dirtmap) {
            std::string mname = "dirtmap";
            const auto& light_vp = rcva_->rendering_resources_->get_vp(mname);
            auto mvp_light = dot2d(light_vp, m);
            CHK(glUniformMatrix4fv(rp.mvp_dirtmap_location, 1, GL_TRUE, (const GLfloat*) mvp_light.flat_begin()));

            CHK(glActiveTexture(GL_TEXTURE0 + 1 + filtered_lights.size()));
            CHK(glBindTexture(GL_TEXTURE_2D, rcva_->rendering_resources_->get_texture(mname, false)));  // false=rgba
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
            CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
            CHK(glActiveTexture(GL_TEXTURE0));

            CHK(glActiveTexture(GL_TEXTURE0 + 2 + filtered_lights.size()));
            CHK(glBindTexture(GL_TEXTURE_2D, rcva_->rendering_resources_->get_texture(cva->material.dirt_texture, false)));  // false=rgba
            if (cva->material.clamp_mode_s == ClampMode::REPEAT) {
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
            } else {
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            }
            if (cva->material.clamp_mode_t == ClampMode::REPEAT) {
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
            } else {
                CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            }
            CHK(glActiveTexture(GL_TEXTURE0));
        }
        if (render_config.cull_faces && cva->material.cull_faces) {
            CHK(glEnable(GL_CULL_FACE));
        }
        switch(cva->material.blend_mode) {
            case BlendMode::OFF:
            case BlendMode::BINARY:
                break;
            case BlendMode::CONTINUOUS:
                CHK(glEnable(GL_BLEND));
                CHK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
                CHK(glDepthMask(GL_FALSE));
                break;
            default:
                throw std::runtime_error("Unknown blend_mode");
        }
        CHK(glDrawArrays(GL_TRIANGLES, 0, 3 * cva->triangles.size()));
        CHK(glDisable(GL_CULL_FACE));
        CHK(glDisable(GL_BLEND));
        CHK(glDepthMask(GL_TRUE));
    }
}

bool RenderableColoredVertexArrayInstance::requires_render_pass() const {
    for(auto cva : triangles_res_subset_) {
        if (cva->material.aggregate_mode == AggregateMode::OFF)
        {
            return true;
        }
    }
    return false;
}

bool RenderableColoredVertexArrayInstance::requires_blending_pass() const {
    for(auto cva : triangles_res_subset_) {
        if ((cva->material.blend_mode == BlendMode::CONTINUOUS) &&
            (cva->material.aggregate_mode == AggregateMode::OFF))
        {
            return true;
        }
    }
    return false;
}

void RenderableColoredVertexArrayInstance::append_sorted_aggregates_to_queue(
    const FixedArray<float, 4, 4>& mvp,
    const FixedArray<float, 4, 4>& m,
    const SceneGraphConfig& scene_graph_config,
    std::list<std::pair<float, std::shared_ptr<ColoredVertexArray>>>& aggregate_queue) const
{
    for(const auto& cva : triangles_res_subset_) {
        if (cva->material.aggregate_mode == AggregateMode::SORTED_CONTINUOUSLY) {
            if ((!cva->material.is_small) ||
                (// (mvp(2, 3) > scene_graph_config.min_distance_small) && // no mvp-check to support rotations
                 (sum(squared(t3_from_4x4(mvp))) > squared(scene_graph_config.min_distance_small)) &&
                 (sum(squared(t3_from_4x4(mvp))) < squared(scene_graph_config.max_distance_small))))
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
    const FixedArray<float, 4, 4>& m,
    const SceneGraphConfig& scene_graph_config,
    std::list<std::shared_ptr<ColoredVertexArray>>& aggregate_queue) const
{
    for(const auto& cva : triangles_res_subset_) {
        if (cva->material.aggregate_mode == AggregateMode::ONCE) {
            aggregate_queue.push_back(std::move(cva->transformed(m)));
        }
    }
}

void RenderableColoredVertexArrayInstance::print_stats() const {
    std::cerr << "#triangle lists: " << triangles_res_subset_.size() << std::endl;
    size_t i = 0;
    for(auto& cva : triangles_res_subset_) {
        std::cerr << "triangle list " << i << " #lines: " << cva->lines.size() << std::endl;
        std::cerr << "triangle list " << i << " #tris:  " << cva->triangles.size() << std::endl;
    }
}
