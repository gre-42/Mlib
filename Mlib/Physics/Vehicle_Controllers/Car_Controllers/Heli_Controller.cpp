#include "Heli_Controller.hpp"
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Vehicle_Domain.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Steering_Type.hpp>
#include <Mlib/Scene_Graph/Animation/Animation_State_Updater.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

HeliController::HeliController(
    RigidBodyVehicle& rb,
    std::map<size_t, float> tire_angles,
    size_t main_rotor_id,
    FixedArray<float, 3> angle_multipliers,
    const PidController<double, double>& height_pid,
    VehicleDomain vehicle_domain)
    : RigidBodyVehicleController{ rb, SteeringType::CAR }
    , height_pid_{ height_pid }
    , tire_angles_{ std::move(tire_angles) }
    , main_rotor_id_{ main_rotor_id }
    , angle_multipliers_{ angle_multipliers }
    , vehicle_domain_{ vehicle_domain }
{
    ascend_to(rb.rbp_.abs_position()(1));
}

HeliController::~HeliController() {
    on_destroy.clear();
}

static const size_t PITCH = 0;
static const size_t YAW = 1;
static const size_t ROLL = 2;

static const auto wheels_name = VariableAndHash<std::string>{"wheels"};
static const auto main_rotor_name = VariableAndHash<std::string>{"main_rotor"};
static const auto tail_rotor_name = VariableAndHash<std::string>{"tail_rotor"};

void HeliController::apply() {
    if (vehicle_domain_ == VehicleDomain::AIR) {
        rb_.set_surface_power(wheels_name, EnginePowerIntent{.surface_power = NAN});  // NAN=break
        for (const auto& [x, _] : tire_angles_) {
            rb_.set_tire_angle_y(x, 0.f);
        }
        rb_.set_surface_power(
            main_rotor_name,
            EnginePowerIntent{
                .surface_power = std::isnan(target_height_)
                    ? 0.f
                    : (float)std::min(0., height_pid_(rb_.rbp_.abs_position()(1) - target_height_))});
        rb_.set_rotor_movement_y(main_rotor_id_, std::isnan(surface_power_)
            ? 0.f
            : angle_multipliers_(PITCH) * sign(surface_power_) * drive_relaxation_);
        float ang = signed_min(steer_angle_ * steer_relaxation_, 45.f * degrees);
        rb_.set_rotor_movement_x(main_rotor_id_, angle_multipliers_(ROLL) * ang);
        rb_.set_surface_power(tail_rotor_name, EnginePowerIntent{.surface_power = angle_multipliers_(YAW) * ang});
    } else if (vehicle_domain_ == VehicleDomain::GROUND) {
        rb_.set_surface_power(wheels_name, EnginePowerIntent{
            .surface_power = surface_power_,
            .drive_relaxation = drive_relaxation_});  // NAN=break
        for (const auto& x : tire_angles_) {
            float ang = signed_min(steer_angle_ * steer_relaxation_, x.second);
            rb_.set_tire_angle_y(x.first, ang);
        }
    } else {
        THROW_OR_ABORT("Unknown vehicle domain");
    }
    if (rb_.animation_state_updater_ != nullptr) {
        rb_.animation_state_updater_->notify_movement_intent();
    }
    RigidBodyVehicleController::apply();
}
