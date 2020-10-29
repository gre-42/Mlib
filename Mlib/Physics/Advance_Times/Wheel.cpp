#include "Wheel.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>

using namespace Mlib;

Wheel::Wheel(RigidBody& rigid_body, AdvanceTimes& advance_times, size_t tire_id, float radius)
: rigid_body_{rigid_body},
  advance_times_{advance_times},
  position_{fixed_nans<float, 3>()},
  rotation_{fixed_nans<float, 3, 3>()},
  tire_id_{tire_id},
  angle_x_{0},
  radius_{radius},
  y0_{NAN}
{}

void Wheel::set_initial_relative_model_matrix(const FixedArray<float, 4, 4>& relative_model_matrix)
{
    position_ = t3_from_4x4(relative_model_matrix);
    rotation_ = R3_from_4x4(relative_model_matrix);
    y0_ = position_(1);
}

void Wheel::set_updated_relative_model_matrix(const FixedArray<float, 4, 4>& relative_model_matrix)
{
    position_ = t3_from_4x4(relative_model_matrix);
}

void Wheel::set_absolute_model_matrix(const FixedArray<float, 4, 4>& absolute_model_matrix)
{
    // do nothing
}

FixedArray<float, 4, 4> Wheel::get_new_relative_model_matrix() const
{
    return assemble_homogeneous_4x4(rotation_, position_);
}

void Wheel::advance_time(float dt) {
    FixedArray<float, 3> tire_angles{fixed_zeros<float, 3>()};
    if (auto it = rigid_body_.tires_.find(tire_id_); it != rigid_body_.tires_.end()) {
        tire_angles(1) = it->second.angle;
        position_(1) = y0_ + it->second.shock_absorber.position();
    }
    angle_x_ += dot0d(rigid_body_.rbi_.v_, rigid_body_.rbi_.abs_z()) * dt * radius_;
    angle_x_ = std::fmod(angle_x_, 2 * M_PI);
    tire_angles(0) = angle_x_;
    rotation_ = tait_bryan_angles_2_matrix(tire_angles);
}

void Wheel::notify_destroyed(void* obj) {
    advance_times_.schedule_delete_advance_time(this);
}
