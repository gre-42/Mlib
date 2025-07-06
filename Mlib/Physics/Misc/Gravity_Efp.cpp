#include "Gravity_Efp.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Fixed_Scaled_Unit_Vector.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Scene_Graph/Instances/Static_World.hpp>

using namespace Mlib;

GravityEfp::GravityEfp(PhysicsEngine& engine)
    : engine_{ engine }
{}

GravityEfp::~GravityEfp() = default;

void GravityEfp::increment_external_forces(
    const PhysicsEngineConfig& cfg,
    const PhysicsPhase& phase,
    const StaticWorld& world)
{
    if (world.gravity == nullptr) {
        return;
    }
    for (auto& rb : engine_.rigid_bodies_.objects()) {
        if (rb.rigid_body->feels_gravity() &&
            (rb.rigid_body->mass() != INFINITY) &&
            phase.group.rigid_bodies.contains(&rb.rigid_body->rbp_))
        {
            auto dt = cfg.dt_substeps(phase);
            rb.rigid_body->rbp_.integrate_delta_v(world.gravity->vector * dt, dt);
        }
    }
}
