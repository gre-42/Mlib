#pragma once
#include <cstddef>
#include <map>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct CollisionTriangleSphere;

class SatTracker {
public:
    void get_collision_plane(
        const std::vector<CollisionTriangleSphere>& triangles0,
        const std::vector<CollisionTriangleSphere>& triangles1,
        double& min_overlap,
        FixedArray<double, 3>& normal) const;
private:
    mutable std::map<
        const std::vector<CollisionTriangleSphere>*,
        std::map<const std::vector<CollisionTriangleSphere>*,
            std::pair<double, FixedArray<double, 3>>>> collision_planes_;
};

}
