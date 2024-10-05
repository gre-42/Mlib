#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct CollisionRidgeSphere;
template <class TData, size_t tnvertices>
struct CollisionPolygonSphere;

class ISurfaceNormal {
public:
    virtual ~ISurfaceNormal() = default;
    virtual FixedArray<float, 3> get_surface_normal(
        const CollisionRidgeSphere& ridge,
        const FixedArray<ScenePos, 3>& position) const = 0;
    virtual FixedArray<float, 3> get_surface_normal(
        const CollisionPolygonSphere<ScenePos, 3>& triangle,
        const FixedArray<ScenePos, 3>& position) const = 0;
    virtual FixedArray<float, 3> get_surface_normal(
        const CollisionPolygonSphere<ScenePos, 4>& quad,
        const FixedArray<ScenePos, 3>& position) const = 0;
};

}
