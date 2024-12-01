#pragma once
#include <Mlib/Billboard_Id.hpp>
#include <Mlib/Geometry/Intersection/Frustum3.hpp>
#include <cmath>
#include <cstdint>

namespace Mlib {

template <class TData, size_t tndim>
class AxisAlignedBoundingBox;
struct Material;
struct Morphology;
struct SceneGraphConfig;
struct ExternalRenderPass;
enum class ExternalRenderPassType;

template <class TData>
class VisibilityCheck {
public:
    explicit VisibilityCheck(const FixedArray<TData, 4, 4>& mvp);
    bool is_visible(
        const std::string& object_name,
        const Material& material,
        const Morphology& morphology,
        BillboardId billboard_id,
        const SceneGraphConfig& scene_graph_config,
        ExternalRenderPassType external_render_pass) const;
    bool is_visible(TData max_center_distance) const;
    bool black_is_visible(
        const Material& material,
        BillboardId billboard_id,
        const SceneGraphConfig& scene_graph_config,
        ExternalRenderPassType external_render_pass) const;
    TData sorting_key(const Material& m) const;
    bool orthographic() const;
    TData distance_squared() const;
    const FixedArray<TData, 4, 4>& mvp() const;
private:
    const FixedArray<TData, 4, 4>& mvp_;
    bool orthographic_;
};

}
