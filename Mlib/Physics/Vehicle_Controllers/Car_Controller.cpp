#include "Car_Controller.hpp"
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Steering_Type.hpp>
#include <Mlib/Scene_Graph/Style_Updater.hpp>
#include <stdexcept>

using namespace Mlib;

CarController::CarController(
    RigidBodyVehicle* rb,
    const std::vector<size_t>& front_tire_ids,
    float max_tire_angle,
    const PidController<float, float>& tire_angle_pid)
: RigidBodyVehicleController{ rb, SteeringType::CAR },
  front_tire_ids_{front_tire_ids},
  max_tire_angle_{max_tire_angle},
  tire_angle_pid_{tire_angle_pid}
{}

CarController::~CarController()
{}

void CarController::apply() {
    rb_->set_surface_power("main", surface_power_);   // NAN=break
    rb_->set_surface_power("breaks", surface_power_); // NAN=break
    float ang0 = signed_min(steer_angle_, max_tire_angle_);
    float ang1 = signed_min(tire_angle_pid_(ang0), max_tire_angle_);
    for (size_t tire_id : front_tire_ids_) {
        rb_->set_tire_angle_y(tire_id, ang1);
    }
    if (rb_->style_updater_ != nullptr) {
        rb_->style_updater_->notify_movement_intent();
    }
}
