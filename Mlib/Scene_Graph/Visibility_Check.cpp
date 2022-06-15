#include "Visibility_Check.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>

using namespace Mlib;

VisibilityCheck::VisibilityCheck(const FixedArray<double, 4, 4>& mvp)
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
    if (bool(external_render_pass.pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK))
    {
        return (m.occluder_pass & external_render_pass.pass) == external_render_pass.pass;
    } else if (has_instances && (m.blend_mode == BlendMode::INVISIBLE)) {
        return false;
    }
    if ((m.aggregate_mode == AggregateMode::OFF) && (m.blend_mode == BlendMode::INVISIBLE)) {
        return false;
    }
    if (external_render_pass.pass == ExternalRenderPassType::DIRTMAP) {
        return true;
    }
    if (external_render_pass.pass == ExternalRenderPassType::STANDARD) {
        if (has_instances) {
            return true;
        }
        bool is_small;
        if (billboard_id == UINT32_MAX) {
            is_small = m.is_small;
        } else {
            is_small = m.billboard_atlas_instance(billboard_id).is_small;
        }
        if (!is_small && std::isnan(max_distance) && (m.distances == default_distances_hard)) {
            return true;
        }
        if (orthographic_) {
            return true;
        }
        double max_dist = std::min(
            std::isnan(max_distance)
                ? scene_graph_config.max_distance_small
                : max_distance,
            m.distances(1));
        double dist2 = distance_squared();
        return (dist2 >= squared(m.distances(0))) && (dist2 <= squared(max_dist));
    }
    throw std::runtime_error("VisibilityCheck::is_visible received unknown render pass type");
}

bool VisibilityCheck::black_is_visible(
    const Material& m,
    uint32_t billboard_id,
    const SceneGraphConfig& scene_graph_config,
    const ExternalRenderPass& external_render_pass) const
{
    if ((external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_GLOBAL_STATIC) ||
        (external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC) ||
        (external_render_pass.pass == ExternalRenderPassType::DIRTMAP))
    {
        return false;
    }
    if (external_render_pass.pass != ExternalRenderPassType::STANDARD) {
        throw std::runtime_error("VisibilityCheck::black_is_visible: unsupported render pass: " + external_render_pass_type_to_string(external_render_pass.pass));
    }
    ExternalRenderPassType occluder_pass = (billboard_id != UINT32_MAX)
        ? m.billboard_atlas_instance(billboard_id).occluder_pass
        : m.occluder_pass;
    if (!bool(occluder_pass & ExternalRenderPassType::LIGHTMAP_BLACK_LOCAL_INSTANCES)) {
        return false;
    }
    if (orthographic_) {
        return true;
    }
    return (distance_squared() <= squared(scene_graph_config.max_distance_black));
}

double VisibilityCheck::sorting_key(const Material& m) const {
    // mvp_ * [0; 0; 0; 1] = position in clip-space,
    // ranging from -1 to +1.
    return -std::abs(mvp_(2, 3) + 1.);
}

bool VisibilityCheck::orthographic() const {
    return orthographic_;
}

double VisibilityCheck::distance_squared() const {
    return sum(squared(t3_from_4x4(mvp_)));
}
