#include "Gravity_Efp.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Fixed_Scaled_Unit_Vector.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Scene_Graph/Instances/Static_World.hpp>

using namespace Mlib;

GravityEfp::GravityEfp() = default;

GravityEfp::~GravityEfp() = default;

void GravityEfp::increment_external_forces(
    const std::list<RigidBodyVehicle*>& olist,
    bool burn_in,
    const PhysicsEngineConfig& cfg,
    const StaticWorld& world)
{
    if (world.gravity == nullptr) {
        return;
    }
    for (auto& rb : olist) {
        if (rb->feels_gravity() && (rb->mass() != INFINITY)) {
            rb->rbp_.integrate_delta_v(world.gravity->vector * cfg.dt_substeps());
        }
    }
}
