#include "Visibility_Check.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>

using namespace Mlib;

VisibilityCheck::VisibilityCheck(const FixedArray<float, 4, 4>& mvp)
: mvp_{mvp},
  orthographic_{(mvp(3, 0) == 0 && mvp(3, 1) == 0 && mvp(3, 2) == 0 && mvp(3, 3) == 1)}
{}

bool VisibilityCheck::is_visible(
    const Material& m,
    uint32_t billboard_id,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    float max_distance,
    bool has_instances) const
{
    if ((external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_GLOBAL_STATIC) ||
        (external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_GLOBAL_DYNAMIC))
    {
        return m.occluder_type != OccluderType::OFF;
    }
    if ((external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_LOCAL_INSTANCES_STATIC) ||
        (external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_NODE_DYNAMIC))
    {
        return m.is_black;
    }
    if (external_render_pass.pass == ExternalRenderPassType::DIRTMAP) {
        return true;
    }
    if (external_render_pass.pass == ExternalRenderPassType::STANDARD) {
        bool is_small;
        if (has_instances) {
            is_small = false;
        } else if (billboard_id == UINT32_MAX) {
            is_small = m.is_small;
        } else {
            if (billboard_id >= m.billboard_atlas_instances.size()) {
                auto tit = m.textures.begin();
                std::string color = (tit == m.textures.end())
                    ? "<no texture>"
                    : tit->texture_descriptor.color;
                throw std::runtime_error(
                    "Billboard ID out of bounds in material \"" + color + "\" (" +
                    std::to_string(billboard_id) +
                    " >= " +
                    std::to_string(m.billboard_atlas_instances.size()) + ')');
            }
            is_small = m.billboard_atlas_instances[billboard_id].is_small;
        }
        if (!is_small && std::isnan(max_distance) && (m.distances == default_distances_hard)) {
            return true;
        }
        if (orthographic_) {
            return false;
        }
        float max_dist = std::min(
            std::isnan(max_distance)
                ? scene_graph_config.max_distance_small
                : max_distance,
            m.distances(1));
        float dist2 = sum(squared(t3_from_4x4(mvp_)));
        return (dist2 >= squared(m.distances(0))) && (dist2 <= squared(max_dist));
    }
    throw std::runtime_error("VisibilityCheck::is_visible received unknown render pass type");
}

float VisibilityCheck::sorting_key(const Material& m) const {
    return (!orthographic_ && ((m.blend_mode & BlendMode::ANY_CONTINUOUS) != 0))
        ? -std::abs(mvp_(2, 3))
        : -INFINITY;
}

bool VisibilityCheck::orthographic() const {
    return orthographic_;
}
