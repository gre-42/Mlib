#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <optional>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct CollisionRidgeSphere;
template <size_t tnvertices>
struct CollisionPolygonSphere;

class ISurfaceNormal {
public:
    virtual ~ISurfaceNormal() = default;
    virtual std::optional<FixedArray<float, 3>> get_surface_normal(
        const CollisionRidgeSphere& ridge,
        const FixedArray<ScenePos, 3>& position) const = 0;
    virtual std::optional<FixedArray<float, 3>> get_surface_normal(
        const CollisionPolygonSphere<3>& triangle,
        const FixedArray<ScenePos, 3>& position) const = 0;
    virtual std::optional<FixedArray<float, 3>> get_surface_normal(
        const CollisionPolygonSphere<4>& quad,
        const FixedArray<ScenePos, 3>& position) const = 0;
};

}
