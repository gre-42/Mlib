#include "Wheel.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Actuators/Tire.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

Wheel::Wheel(
    RigidBodyVehicle& rigid_body,
    AdvanceTimes& advance_times,
    size_t tire_id,
    float radius)
    : rigid_body_{ rigid_body }
    , advance_times_{ advance_times }
    , transformation_matrix_{
          fixed_nans<float, 3, 3>(),
          fixed_nans<ScenePos, 3>() }
          , tire_id_{ tire_id }
    , angle_x_{ 0 }
    , radius_{ radius }
    , y0_{ NAN }
{}

Wheel::~Wheel() {
    on_destroy.clear();
}

void Wheel::set_initial_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix)
{
    transformation_matrix_ = relative_model_matrix;
    y0_ = (float)transformation_matrix_.t(1);
}

void Wheel::set_updated_relative_model_matrix(const TransformationMatrix<float, ScenePos, 3>& relative_model_matrix)
{
    transformation_matrix_.t = relative_model_matrix.t;
}

void Wheel::set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix)
{
    // do nothing
}

TransformationMatrix<float, ScenePos, 3> Wheel::get_new_relative_model_matrix() const
{
    return transformation_matrix_;
}

void Wheel::advance_time(float dt, const StaticWorld& world) {
    FixedArray<float, 3> tire_angles{fixed_zeros<float, 3>()};
    if (auto t = rigid_body_.tires_.try_get(tire_id_); t != nullptr) {
        tire_angles(1) = t->angle_y;
        transformation_matrix_.t(1) = y0_ + t->shock_absorber_position;
        angle_x_ = t->angle_x;
    }
    tire_angles(0) = angle_x_;
    transformation_matrix_.R = tait_bryan_angles_2_matrix(tire_angles);
}

void Wheel::notify_destroyed(SceneNode& destroyed_object) {
    if (destroyed_object.has_relative_movable()) {
        if (&destroyed_object.get_relative_movable() != this) {
            verbose_abort("Unexpected relative movable");
        }
        destroyed_object.clear_relative_movable();
    }
    global_object_pool.remove(this);
}
