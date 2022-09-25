#pragma once
#include <cstddef>
#include <cstdint>

namespace Mlib {

template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
template <class TData>
class Frustum3;
struct Material;
struct SceneGraphConfig;
enum class ExternalRenderPassType;
template <class TData>
class VisibilityCheck;

template <class TData>
bool is_visible(
    const VisibilityCheck<TData>& vc,
    const Material& m,
    uint32_t billboard_id,
    const SceneGraphConfig& scene_graph_config,
    ExternalRenderPassType external_render_pass,
    const Frustum3<TData>* frustum,
    const AxisAlignedBoundingBox<TData, 3>* aabb);

}
