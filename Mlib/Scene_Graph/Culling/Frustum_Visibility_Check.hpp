#pragma once
#include <Mlib/Billboard_Id.hpp>
#include <cstddef>
#include <cstdint>
#include <string>

namespace Mlib {

template <class TData>
class VisibilityCheck;
struct Material;
struct Morphology;
struct SceneGraphConfig;
enum class ExternalRenderPassType;
template <class TData, size_t tndim>
class ExtremalAxisAlignedBoundingBox;
template <class TData>
class Frustum3;

template <class TData>
class FrustumVisibilityCheck {
public:
    explicit FrustumVisibilityCheck(const VisibilityCheck<TData>& vc);
    bool is_visible(
        const std::string& object_name,
        const Material& material,
        const Morphology& morphology,
        BillboardId billboard_id,
        const SceneGraphConfig& scene_graph_config,
        ExternalRenderPassType external_render_pass,
        const ExtremalAxisAlignedBoundingBox<TData, 3>& aabb) const;
private:
    const VisibilityCheck<TData>& vc_;
    Frustum3<TData> frustum_;
};

}
