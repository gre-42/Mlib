#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Scene_Precision.hpp>

namespace Mlib {

struct PhysicsEngineConfig;

class RidgeIntersectionPointsBvh {
public:
    RidgeIntersectionPointsBvh(const PhysicsEngineConfig& cfg);
    void insert(const FixedArray<ScenePos, 3>& intersection_point);
    bool has_neighbor(const FixedArray<ScenePos, 3>& intersection_point);
private:
    Bvh<CompressedScenePos, 3, FixedArray<CompressedScenePos, 3>> bvh_;
    ScenePos radius_;
};

}
