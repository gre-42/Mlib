#pragma once
#include <map>
#include <memory>
#include <vector>

namespace Mlib {

template <class TData, size_t n>
class PlaneNd;
struct CollisionTriangleSphere;

class SatTracker {
public:
    void get_collision_plane(
        const std::vector<CollisionTriangleSphere>& triangles0,
        const std::vector<CollisionTriangleSphere>& triangles1,
        double& min_overlap,
        PlaneNd<double, 3>& plane) const;
private:
    mutable std::map<
        const std::vector<CollisionTriangleSphere>*,
        std::map<const std::vector<CollisionTriangleSphere>*,
            std::pair<double, PlaneNd<double, 3>>>> collision_planes_;
};

}
