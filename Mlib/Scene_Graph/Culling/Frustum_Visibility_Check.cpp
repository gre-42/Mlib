#include "Frustum_Visibility_Check.hpp"
#include <Mlib/Scene_Graph/Culling/Is_Visible.hpp>
#include <Mlib/Scene_Graph/Culling/Visibility_Check.hpp>

using namespace Mlib;

template <class TData>
FrustumVisibilityCheck<TData>::FrustumVisibilityCheck(const VisibilityCheck<TData>& vc)
    : vc_{ vc }
    , frustum_{ Frustum3<TData>::from_projection_matrix(vc.mvp()) }
{}

template <class TData>
bool FrustumVisibilityCheck<TData>::is_visible(
    const std::string& object_name,
    const Material& material,
    const Morphology& morphology,
    BillboardId billboard_id,
    const SceneGraphConfig& scene_graph_config,
    ExternalRenderPassType external_render_pass,
    const ExtremalAxisAlignedBoundingBox<TData, 3>& aabb) const
{
    return Mlib::is_visible<TData>(vc_, object_name, material, morphology, billboard_id, scene_graph_config, external_render_pass, &frustum_, &aabb);
}

namespace Mlib {
    template class FrustumVisibilityCheck<float>;
    template class FrustumVisibilityCheck<double>;
}
