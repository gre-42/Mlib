#include "Tank_Controller.hpp"
#include <Mlib/Physics/Actuators/Engine_Power_Delta_Intent.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Steering_Type.hpp>
#include <Mlib/Scene_Graph/Animation/Animation_State_Updater.hpp>
#include <stdexcept>

using namespace Mlib;

TankController::TankController(
    RigidBodyVehicle& rb,
    const std::vector<size_t>& left_tires,
    const std::vector<size_t>& right_tires,
    float steering_multiplier)
: RigidBodyVehicleController{ rb, SteeringType::TANK },
  left_tires_{ left_tires },
  right_tires_{ right_tires },
  delta_power_{ steering_multiplier }
{}

TankController::~TankController()
{}

static const auto main_name = VariableAndHash<std::string>{"main"};
static const auto left_name = VariableAndHash<std::string>{"left"};
static const auto right_name = VariableAndHash<std::string>{"right"};

void TankController::apply() {
    if (std::isnan(surface_power_)) {
        rb_.set_surface_power(main_name, EnginePowerIntent{.surface_power = NAN});
        rb_.set_delta_surface_power(left_name, EnginePowerDeltaIntent::zero());
        rb_.set_delta_surface_power(right_name, EnginePowerDeltaIntent::zero());
    } else {
        float delta_relaxation = std::min(
            steer_relaxation_,
            std::min(std::abs(steer_angle_) / (45.f * degrees), 1.f));
        rb_.set_surface_power(main_name,
            EnginePowerIntent{
                .surface_power = surface_power_,
                .drive_relaxation = drive_relaxation_});
        rb_.set_delta_surface_power(left_name,
            EnginePowerDeltaIntent{
                .delta_power = -sign(steer_angle_) * delta_power_,
                .delta_relaxation = delta_relaxation});
        rb_.set_delta_surface_power(right_name,
            EnginePowerDeltaIntent{
                .delta_power = +sign(steer_angle_) * delta_power_,
                .delta_relaxation = delta_relaxation});
    }
    if (rb_.animation_state_updater_ != nullptr) {
        rb_.animation_state_updater_->notify_movement_intent();
    }
    RigidBodyVehicleController::apply();
}
