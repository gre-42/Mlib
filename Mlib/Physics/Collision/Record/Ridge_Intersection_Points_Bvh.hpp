#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>

namespace Mlib {

struct PhysicsEngineConfig;

class RidgeIntersectionPointsBvh {
public:
    RidgeIntersectionPointsBvh(const PhysicsEngineConfig& cfg);
    void insert(const FixedArray<double, 3>& intersection_point);
    bool has_neighbor(const FixedArray<double, 3>& intersection_point);
private:
    Bvh<double, FixedArray<double, 3>, 3> bvh_;
    double radius_;
};

}
