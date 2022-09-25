#pragma once
#include <Mlib/Geometry/Intersection/Frustum3.hpp>
#include <cmath>
#include <cstdint>

namespace Mlib {

template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
struct Material;
struct SceneGraphConfig;
struct ExternalRenderPass;
enum class ExternalRenderPassType;

class VisibilityCheck {
public:
    explicit VisibilityCheck(const FixedArray<double, 4, 4>& mvp);
    bool is_visible(
        const Material& m,
        uint32_t billboard_id,
        const SceneGraphConfig& scene_graph_config,
        ExternalRenderPassType external_render_pass,
        const AxisAlignedBoundingBox<double, 3>& aabb) const;
    bool is_visible(double max_center_distance) const;
    bool black_is_visible(
        const Material& m,
        uint32_t billboard_id,
        const SceneGraphConfig& scene_graph_config,
        ExternalRenderPassType external_render_pass) const;
    static bool instances_are_visible(
        const Material& m,
        ExternalRenderPassType external_render_pass);
    double sorting_key(const Material& m) const;
    bool orthographic() const;
private:
    double distance_squared() const;
    const FixedArray<double, 4, 4>& mvp_;
    bool orthographic_;
    Frustum3<double> frustum_;
};

}
