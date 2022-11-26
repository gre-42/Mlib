#include "Tank_Controller.hpp"
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Steering_Type.hpp>
#include <Mlib/Scene_Graph/Animation_State_Updater.hpp>
#include <stdexcept>

using namespace Mlib;

TankController::TankController(
    RigidBodyVehicle* rb,
    const std::vector<size_t>& left_tires,
    const std::vector<size_t>& right_tires,
    float steering_multiplier)
: RigidBodyVehicleController{ rb, SteeringType::TANK },
  left_tires_{ left_tires },
  right_tires_{ right_tires },
  steering_multiplier_{ steering_multiplier }
{}

TankController::~TankController()
{}

void TankController::apply() {
    if (std::isnan(surface_power_)) {
        rb_->set_surface_power("left", EnginePowerIntent{.surface_power = NAN});
        rb_->set_surface_power("right", EnginePowerIntent{.surface_power = NAN});
    } else {
        float angle = signed_min(steer_angle_, 45.f * degrees);
        rb_->set_surface_power("left",
            EnginePowerIntent{
                .surface_power = surface_power_,
                .delta_power = -angle * steering_multiplier_});
        rb_->set_surface_power("right",
            EnginePowerIntent{
                .surface_power = surface_power_,
                .delta_power = +angle * steering_multiplier_});
    }
    if (rb_->animation_state_updater_ != nullptr) {
        rb_->animation_state_updater_->notify_movement_intent();
    }
}
