#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <cstddef>
#include <optional>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TPosition>
struct CollisionRidgeSphere;
template <class TPosition, size_t tnvertices>
struct CollisionPolygonSphere;

class ISurfaceNormal {
public:
    virtual ~ISurfaceNormal() = default;
    virtual std::optional<FixedArray<float, 3>> get_surface_normal(
        const CollisionRidgeSphere<CompressedScenePos>& ridge,
        const FixedArray<ScenePos, 3>& position) const = 0;
    virtual std::optional<FixedArray<float, 3>> get_surface_normal(
        const CollisionPolygonSphere<CompressedScenePos, 3>& triangle,
        const FixedArray<ScenePos, 3>& position) const = 0;
    virtual std::optional<FixedArray<float, 3>> get_surface_normal(
        const CollisionPolygonSphere<CompressedScenePos, 4>& quad,
        const FixedArray<ScenePos, 3>& position) const = 0;
};

}
