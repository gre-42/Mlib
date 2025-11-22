#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <cstddef>
#include <map>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TPosition, size_t tnvertices>
struct CollisionPolygonSphere;
class IIntersectableMesh;

void get_overlap(
    const IIntersectableMesh& mesh0,
    const IIntersectableMesh& mesh1,
    ScenePos& min_overlap,
    FixedArray<SceneDir, 3>& normal);

class SatTracker {
public:
    void get_collision_plane(
        const IIntersectableMesh& mesh0,
        const IIntersectableMesh& mesh1,
        ScenePos& min_overlap,
        FixedArray<SceneDir, 3>& normal) const;
private:
    mutable std::map<
        const IIntersectableMesh*,
        std::map<const IIntersectableMesh*,
            std::pair<ScenePos, FixedArray<SceneDir, 3>>>> collision_planes_;
};

}
