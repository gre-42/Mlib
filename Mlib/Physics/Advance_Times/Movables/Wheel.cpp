#include "Wheel.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>

using namespace Mlib;

Wheel::Wheel(
    RigidBodyVehicle& rigid_body,
    AdvanceTimes& advance_times,
    size_t tire_id,
    float radius)
: rigid_body_{rigid_body},
  advance_times_{advance_times},
  transformation_matrix_{
      fixed_nans<float, 3, 3>(),
      fixed_nans<float, 3>()},
  tire_id_{tire_id},
  angle_x_{0},
  radius_{radius},
  y0_{NAN}
{}

Wheel::~Wheel()
{}

void Wheel::set_initial_relative_model_matrix(const TransformationMatrix<float, 3>& relative_model_matrix)
{
    transformation_matrix_ = relative_model_matrix;
    y0_ = transformation_matrix_.t()(1);
}

void Wheel::set_updated_relative_model_matrix(const TransformationMatrix<float, 3>& relative_model_matrix)
{
    transformation_matrix_.t() = relative_model_matrix.t();
}

void Wheel::set_absolute_model_matrix(const TransformationMatrix<float, 3>& absolute_model_matrix)
{
    // do nothing
}

TransformationMatrix<float, 3> Wheel::get_new_relative_model_matrix() const
{
    return transformation_matrix_;
}

void Wheel::advance_time(float dt) {
    FixedArray<float, 3> tire_angles{fixed_zeros<float, 3>()};
    if (auto it = rigid_body_.tires_.find(tire_id_); it != rigid_body_.tires_.end()) {
        tire_angles(1) = it->second.angle_y;
        transformation_matrix_.t()(1) = y0_ + it->second.shock_absorber_position;
        angle_x_ = it->second.angle_x;
    }
    tire_angles(0) = angle_x_;
    transformation_matrix_.R() = tait_bryan_angles_2_matrix(tire_angles);
}

void Wheel::notify_destroyed(void* obj) {
    advance_times_.schedule_delete_advance_time(this);
}
