#include "Rigid_Body_Vehicle_Controller.hpp"
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Threads/Recursion_Guard.hpp>
#include <Mlib/Threads/Thread_Local.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>

using namespace Mlib;

RigidBodyVehicleController::RigidBodyVehicleController(
    RigidBodyVehicle& rb,
    SteeringType steering_type)
    : steering_type{ steering_type }
    , rb_{ rb }
    , surface_power_{ NAN }
    , drive_relaxation_{ 0.f }
    , steer_angle_{ NAN }
    , steer_relaxation_{ 0.f }
    , target_height_{ NAN }
    , trailer_{ nullptr }
{}

RigidBodyVehicleController::~RigidBodyVehicleController() = default;

void RigidBodyVehicleController::step_on_brakes(float relaxation) {
    if (relaxation < drive_relaxation_) {
        return;
    }
    surface_power_ = NAN;
    drive_relaxation_ = relaxation;
    if (trailer_ != nullptr) {
        static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
        RecursionGuard rg{recursion_counter};
        trailer_->step_on_brakes(relaxation);
    }
}

void RigidBodyVehicleController::drive(float surface_power, float relaxation) {
    if (relaxation < drive_relaxation_) {
        return;
    }
    surface_power_ = surface_power;
    drive_relaxation_ = relaxation;
    if (trailer_ != nullptr) {
        static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
        RecursionGuard rg{recursion_counter};
        trailer_->drive(surface_power, relaxation);
    }
}

void RigidBodyVehicleController::roll_tires() {
    surface_power_ = 0.f;
    if (trailer_ != nullptr) {
        static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
        RecursionGuard rg{recursion_counter};
        trailer_->roll_tires();
    }
}

void RigidBodyVehicleController::steer(float angle, float relaxation) {
    if (relaxation < steer_relaxation_) {
        return;
    }
    steer_angle_ = angle;
    steer_relaxation_ = relaxation;
    if (trailer_ != nullptr) {
        static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
        RecursionGuard rg{recursion_counter};
        trailer_->steer(angle, relaxation);
    }
}

void RigidBodyVehicleController::set_stearing_wheel_amount(float left_amount, float relaxation) {
    steer(left_amount * 45.f * degrees, relaxation);
}

void RigidBodyVehicleController::ascend_to(double target_height) {
    target_height_ = target_height;
    if (trailer_ != nullptr) {
        static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
        RecursionGuard rg{recursion_counter};
        trailer_->ascend_to(target_height);
    }
}

void RigidBodyVehicleController::ascend_by(double delta_height) {
    if (std::isnan(target_height_)) {
        return;
    }
    target_height_ += delta_height;
    if (trailer_ != nullptr) {
        static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
        RecursionGuard rg{recursion_counter};
        trailer_->ascend_by(delta_height);
    }
}

void RigidBodyVehicleController::reset_parameters(
    float surface_power,
    float steer_angle)
{
    surface_power_ = surface_power;
    steer_angle_ = steer_angle;
    if (trailer_ != nullptr) {
        static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
        RecursionGuard rg{recursion_counter};
        trailer_->reset_parameters(surface_power, steer_angle);
    }
}

void RigidBodyVehicleController::reset_relaxation(
    float drive_relaxation,
    float steer_relaxation)
{
    drive_relaxation_ = drive_relaxation;
    steer_relaxation_ = steer_relaxation;
    if (trailer_ != nullptr) {
        static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
        RecursionGuard rg{recursion_counter};
        trailer_->reset_relaxation(drive_relaxation, steer_relaxation);
    }
}

void RigidBodyVehicleController::set_trailer(
    const DanglingBaseClassRef<RigidBodyVehicleController>& trailer)
{
    if (trailer_ != nullptr) {
        THROW_OR_ABORT("Trailer already set (0)");
    }
    if (on_destroy_trailer_.has_value()) {
        THROW_OR_ABORT("Trailer already set (1)");
    }
    on_destroy_trailer_.emplace(trailer->on_destroy, CURRENT_SOURCE_LOCATION);
    on_destroy_trailer_->add([this](){
        trailer_ = nullptr;
        on_destroy_trailer_.reset();
    }, CURRENT_SOURCE_LOCATION);
    trailer_ = trailer.ptr();
}

void RigidBodyVehicleController::apply() {
    if (trailer_ != nullptr) {
        static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
        RecursionGuard rg{recursion_counter};
        trailer_->apply();
    }
}
