#include "Is_Visible.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

template <class TData>
bool Mlib::is_visible(
    const VisibilityCheck<TData>& vc,
    const Material& m,
    uint32_t billboard_id,
    const SceneGraphConfig& scene_graph_config,
    ExternalRenderPassType external_render_pass,
    const Frustum3<TData>* frustum,
    const AxisAlignedBoundingBox<TData, 3>* aabb)
{
    assert_true((billboard_id != UINT32_MAX) || m.billboard_atlas_instances.empty());
    if (any(external_render_pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK) ||
        any(external_render_pass & ExternalRenderPassType::DIRTMAP_MASK))
    {
        ExternalRenderPassType occluder_pass = (billboard_id != UINT32_MAX)
            ? m.billboard_atlas_instance(billboard_id).occluder_pass
            : m.occluder_pass;
        return (occluder_pass & external_render_pass) == external_render_pass;
    }
    if ((m.aggregate_mode == AggregateMode::NONE) && (m.blend_mode == BlendMode::INVISIBLE)) {
        return false;
    }
    if (any(external_render_pass & ExternalRenderPassType::STANDARD_OR_IMPOSTER_NODE)) {
        if (vc.orthographic()) {
            return true;
        }
        TData max_center_distance;
        if (billboard_id == UINT32_MAX) {
            max_center_distance = m.center_distances(1);
        } else {
            max_center_distance = m.billboard_atlas_instance(billboard_id).max_center_distance;
        }
        TData dist2 = vc.distance_squared();
        if (!((dist2 >= squared(m.center_distances(0))) && (dist2 <= squared(max_center_distance)))) {
            return false;
        }
        if (!frustum != !aabb) {
            THROW_OR_ABORT("Inconsistent frustum and AABB NAN-ness");
        }
        if (frustum == nullptr) {
            return true;
        } else {
            return frustum->contains(*aabb);
        }
    }
    THROW_OR_ABORT("VisibilityCheck::is_visible received unknown render pass type");
}

template bool Mlib::is_visible<float>(
    const VisibilityCheck<float>& vc,
    const Material& m,
    uint32_t billboard_id,
    const SceneGraphConfig& scene_graph_config,
    ExternalRenderPassType external_render_pass,
    const Frustum3<float>* frustum,
    const AxisAlignedBoundingBox<float, 3>* aabb);

template bool Mlib::is_visible<double>(
    const VisibilityCheck<double>& vc,
    const Material& m,
    uint32_t billboard_id,
    const SceneGraphConfig& scene_graph_config,
    ExternalRenderPassType external_render_pass,
    const Frustum3<double>* frustum,
    const AxisAlignedBoundingBox<double, 3>* aabb);
