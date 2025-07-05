#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

class RigidBodyVehicle;
struct PhysicsEngineConfig;
struct PhysicsPhase;
struct TirePowerIntent;

void handle_tire_triangle_intersection(
    const TirePowerIntent& P,
    RigidBodyVehicle& rb,
    const FixedArray<float, 3>& v_street,
    const FixedArray<float, 3>& vc_street,
    const FixedArray<float, 3>& vc,
    const FixedArray<float, 3>& n3,
    float v0,
    const FixedArray<float, 3>& surface_normal,
    const PhysicsEngineConfig& cfg,
    const PhysicsPhase& phase,
    size_t tire_id,
    float& force_min,
    float& force_max);

}
