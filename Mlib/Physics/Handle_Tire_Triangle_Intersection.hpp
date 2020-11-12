#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

class RigidBody;
struct PhysicsEngineConfig;

FixedArray<float, 3> handle_tire_triangle_intersection(
    RigidBody& rb,
    const FixedArray<float, 3>& v3,
    const FixedArray<float, 3>& n3,
    float force_n1,
    const PhysicsEngineConfig& cfg,
    size_t tire_id);

}
