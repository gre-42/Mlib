#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

namespace Mlib {

template <class TData>
class VisibilityCheck;
struct Material;
struct SceneGraphConfig;
enum class ExternalRenderPassType;
template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
template <class TData>
class Frustum3;

template <class TData>
class FrustumVisibilityCheck {
public:
    explicit FrustumVisibilityCheck(const VisibilityCheck<TData>& vc);
    bool is_visible(
        const std::string& object_name,
        const Material& m,
        uint32_t billboard_id,
        const SceneGraphConfig& scene_graph_config,
        ExternalRenderPassType external_render_pass,
        const AxisAlignedBoundingBox<TData, 3>& aabb) const;
private:
    const VisibilityCheck<TData>& vc_;
    Frustum3<TData> frustum_;
};

}
