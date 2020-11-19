#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

class RigidBody;
struct PhysicsEngineConfig;
struct PowerIntent;

FixedArray<float, 3> updated_tire_speed(
    const PowerIntent& P,
    RigidBody& rb,
    const FixedArray<float, 3>& v3,
    const FixedArray<float, 3>& n3,
    const FixedArray<float, 3>& surface_normal,
    float force_n1,
    const PhysicsEngineConfig& cfg,
    size_t tire_id);

FixedArray<float, 3> handle_tire_triangle_intersection(
    RigidBody& rb,
    const FixedArray<float, 3>& v3,
    const FixedArray<float, 3>& n3,
    const FixedArray<float, 3>& surface_normal,
    float force_n1,
    const PhysicsEngineConfig& cfg,
    size_t tire_id);

}
