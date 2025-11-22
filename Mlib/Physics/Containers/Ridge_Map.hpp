#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Physics/Containers/Elements/Collision_Ridge_Sphere.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <map>

namespace Mlib {

using RidgeMap = std::map<
    std::pair<OrderableFixedArray<CompressedScenePos, 3>, OrderableFixedArray<CompressedScenePos, 3>>,
    RigidBodyAndCollisionRidgeSphere<CompressedScenePos>>;

}
