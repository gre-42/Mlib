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
        if (orthographic_) {
            return true;
        }
        double max_center_distance;
        if (billboard_id == UINT32_MAX) {
            max_center_distance = m.center_distances(1);
        } else {
            max_center_distance = m.billboard_atlas_instance(billboard_id).max_center_distance;
        }
        double dist2 = distance_squared();
        return (dist2 >= squared(m.center_distances(0))) && (dist2 <= squared(max_center_distance));
    }
    throw std::runtime_error("VisibilityCheck::is_visible received unknown render pass type");
}

bool VisibilityCheck::is_visible(double max_center_distance) const
{
    if (orthographic_) {
        return true;
    }
    return (distance_squared() <= squared(max_center_distance)); 
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
    if (orthographic_) {
        throw std::runtime_error("\"distance_squared()\" called on orthogonal camera");
    }
    return sum(squared(t3_from_4x4(mvp_)));
}
