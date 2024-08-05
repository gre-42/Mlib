#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Scene_Pos.hpp>

namespace Mlib {

struct PhysicsEngineConfig;

class RidgeIntersectionPointsBvh {
public:
    RidgeIntersectionPointsBvh(const PhysicsEngineConfig& cfg);
    void insert(const FixedArray<ScenePos, 3>& intersection_point);
    bool has_neighbor(const FixedArray<ScenePos, 3>& intersection_point);
private:
    Bvh<ScenePos, FixedArray<ScenePos, 3>, 3> bvh_;
    ScenePos radius_;
};

}
