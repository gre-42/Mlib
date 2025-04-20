#include "Is_Visible.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Intersection/Extremal_Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Morphology.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>
#include <Mlib/Scene_Graph/Interfaces/IRenderable_Hider.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

template <class TData>
bool Mlib::is_visible(
    const VisibilityCheck<TData>& vc,
    const std::string& object_name,
    const Material& material,
    const Morphology& morphology,
    BillboardId billboard_id,
    const SceneGraphConfig& scene_graph_config,
    ExternalRenderPassType external_render_pass,
    const Frustum3<TData>* frustum,
    const ExtremalAxisAlignedBoundingBox<TData, 3>* aabb)
{
    assert_true((billboard_id != BILLBOARD_ID_NONE) || material.billboard_atlas_instances.empty());
    if ((scene_graph_config.renderable_hider != nullptr) &&
        !scene_graph_config.renderable_hider->is_visible(object_name))
    {
        return false;
    }
    if (any(external_render_pass & ExternalRenderPassType::LIGHTMAP_ANY_MASK) ||
        any(external_render_pass & ExternalRenderPassType::DIRTMAP_MASK))
    {
        ExternalRenderPassType occluder_pass = material.get_occluder_pass(billboard_id, object_name);
        return (occluder_pass & external_render_pass) == external_render_pass;
    }
    if (material.blend_mode == BlendMode::INVISIBLE) {
        return false;
    }
    if (any(external_render_pass & ExternalRenderPassType::IMPOSTER_NODE)) {
        return morphology.center_distances2(1) == INFINITY;
    } else if (any(external_render_pass & ExternalRenderPassType::ZOOM_NODE)) {
        return morphology.center_distances2(0) == 0.f;
    } else if (any(external_render_pass & ExternalRenderPassType::STANDARD)) {
        if (vc.orthographic()) {
            return true;
        }
        TData max_center_distance2 = (TData)material.max_center_distance2(billboard_id, morphology, object_name);
        TData dist2 = vc.distance_squared();
        if (!((dist2 >= morphology.center_distances2(0)) && (dist2 < max_center_distance2))) {
            return false;
        }
        if (!frustum != !aabb) {
            THROW_OR_ABORT("Inconsistent frustum and AABB NAN-ness");
        }
        if (frustum == nullptr) {
            return true;
        } else {
            return aabb->full() || frustum->intersects(aabb->data());
        }
    }
    THROW_OR_ABORT("VisibilityCheck::is_visible received unknown render pass type");
}

template bool Mlib::is_visible<float>(
    const VisibilityCheck<float>& vc,
    const std::string& object_name,
    const Material& material,
    const Morphology& morphology,
    BillboardId billboard_id,
    const SceneGraphConfig& scene_graph_config,
    ExternalRenderPassType external_render_pass,
    const Frustum3<float>* frustum,
    const ExtremalAxisAlignedBoundingBox<float, 3>* aabb);

template bool Mlib::is_visible<double>(
    const VisibilityCheck<double>& vc,
    const std::string& object_name,
    const Material& material,
    const Morphology& morphology,
    BillboardId billboard_id,
    const SceneGraphConfig& scene_graph_config,
    ExternalRenderPassType external_render_pass,
    const Frustum3<double>* frustum,
    const ExtremalAxisAlignedBoundingBox<double, 3>* aabb);
