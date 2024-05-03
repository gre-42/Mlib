#include "Plane_As_Car_Controller.hpp"
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Vehicle_Domain.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Steering_Type.hpp>
#include <Mlib/Scene_Graph/Animation/Animation_State_Updater.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

PlaneAsCarController::PlaneAsCarController(
    RigidBodyVehicle& rb,
    const std::map<size_t, float>& tire_angles)
    : RigidBodyVehicleController{ rb, SteeringType::CAR }
    , tire_angles_{ tire_angles }
{
    ascend_to(rb.rbp_.abs_position()(1));
}

PlaneAsCarController::~PlaneAsCarController()
{}

void PlaneAsCarController::apply() {
    auto forward = [this](){
        for (const auto& x : tire_angles_) {
            rb_.set_tire_angle_y(x.first, 0.f);
        }};
    auto steer = [this](){
        for (const auto& x : tire_angles_) {
            float ang = signed_min(steer_angle_, x.second);
            rb_.set_tire_angle_y(x.first, ang);
        }};
    switch (rb_.current_vehicle_domain_) {
    case VehicleDomain::AIR:
    case VehicleDomain::UNDEFINED:
        forward();
        rb_.set_surface_power("wheels", EnginePowerIntent{ .surface_power = NAN });  // NAN=break
        rb_.set_surface_power("turbine", EnginePowerIntent{.surface_power = surface_power_});
        return;
    case VehicleDomain::GROUND:
        steer();
        rb_.set_surface_power("wheels", EnginePowerIntent{.surface_power = 0.f});
        rb_.set_surface_power("turbine", EnginePowerIntent{.surface_power = surface_power_});
        return;
    case VehicleDomain::END:
        THROW_OR_ABORT("Invalid vehicle domain");
    }
    THROW_OR_ABORT("Unknown vehicle domain");
    if (rb_.animation_state_updater_ != nullptr) {
        rb_.animation_state_updater_->notify_movement_intent();
    }
    RigidBodyVehicleController::apply();
}
