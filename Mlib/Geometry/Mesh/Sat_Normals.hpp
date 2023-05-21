#pragma once
#include <cstddef>
#include <map>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct CollisionTriangleSphere;
class IIntersectableMesh;

class SatTracker {
public:
    void get_collision_plane(
        const IIntersectableMesh& mesh0,
        const IIntersectableMesh& mesh1,
        double& min_overlap,
        FixedArray<double, 3>& normal) const;
private:
    mutable std::map<
        const IIntersectableMesh*,
        std::map<const IIntersectableMesh*,
            std::pair<double, FixedArray<double, 3>>>> collision_planes_;
};

}
