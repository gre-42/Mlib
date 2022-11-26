#include "Car_Controller.hpp"
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Steering_Type.hpp>
#include <Mlib/Scene_Graph/Animation_State_Updater.hpp>
#include <stdexcept>

using namespace Mlib;

CarController::CarController(
    RigidBodyVehicle* rb,
    const std::vector<size_t>& front_tire_ids,
    float max_tire_angle,
    PhysicsEngine& physics_engine)
: RigidBodyVehicleController{ rb, SteeringType::CAR },
  front_tire_ids_{front_tire_ids},
  max_tire_angle_{max_tire_angle},
  applied_{false},
  physics_engine_{physics_engine}
{
    physics_engine_.add_controllable(this);
}

CarController::~CarController()
{
    physics_engine_.remove_controllable(this);
}

void CarController::apply() {
    if (applied_) {
        throw std::runtime_error("Car controller already applied");
    }
    applied_ = true;
    rb_->set_surface_power("main", EnginePowerIntent{.surface_power = surface_power_, .relaxation = relaxation_});   // NAN=break
    rb_->set_surface_power("brakes", EnginePowerIntent{.surface_power = surface_power_, .relaxation = relaxation_}); // NAN=break
    if (!front_tire_ids_.empty()) {
        float ang = signed_min(steer_angle_, max_tire_angle_);
        for (size_t tire_id : front_tire_ids_) {
            rb_->set_tire_angle_y(tire_id, ang);
        }
    }
    if (rb_->animation_state_updater_ != nullptr) {
        rb_->animation_state_updater_->notify_movement_intent();
    }
}

void CarController::notify_reset(bool burn_in, const PhysicsEngineConfig& cfg) {
    applied_ = false;
}
