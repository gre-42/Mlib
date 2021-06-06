#include "Visibility_Check.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
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
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass,
    float max_distance) const
{
    if ((external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_TO_TEXTURE) && (m.occluder_type != OccluderType::OFF)) {
        return true;
    }
    if (!m.is_small && std::isnan(max_distance) && (m.distances == default_distances_hard)) {
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

float VisibilityCheck::sorting_key(const Material& m, const SceneGraphConfig& scene_graph_config) const {
    return ((m.blend_mode == BlendMode::CONTINUOUS) && !orthographic_)
        ? -mvp_(2, 3)
        : -INFINITY;
}

bool VisibilityCheck::orthographic() const {
    return orthographic_;
}
