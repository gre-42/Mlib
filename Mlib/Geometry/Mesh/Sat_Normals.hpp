#pragma once
#include <Mlib/Scene_Pos.hpp>
#include <cstddef>
#include <map>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TData, size_t tnvertices>
struct CollisionPolygonSphere;
class IIntersectableMesh;

class SatTracker {
public:
    void get_collision_plane(
        const IIntersectableMesh& mesh0,
        const IIntersectableMesh& mesh1,
        ScenePos& min_overlap,
        FixedArray<ScenePos, 3>& normal) const;
private:
    mutable std::map<
        const IIntersectableMesh*,
        std::map<const IIntersectableMesh*,
            std::pair<ScenePos, FixedArray<ScenePos, 3>>>> collision_planes_;
};

}
