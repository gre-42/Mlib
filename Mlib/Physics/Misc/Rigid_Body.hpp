#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Collision/Resolve_Collision_Type.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Integrator.hpp>
#include <Mlib/Physics/Misc/Tire.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <map>
#include <memory>
#include <mutex>

namespace Mlib {

class RigidBodies;
class RigidBodyEngine;
struct Beacon;
class Damageable;
class Player;

/**
 * From: https://en.wikipedia.org/wiki/Torque#Definition_and_relation_to_angular_momentum
 */
class RigidBody: public DestructionObserver, public AbsoluteMovable, public StatusWriter {
public:
    RigidBody(
        RigidBodies& rigid_bodies,
        const RigidBodyIntegrator& rbi,
        const TransformationMatrix<double, 3>* geographic_mapping = nullptr);
    ~RigidBody();
    void reset_forces();
    void integrate_force(const VectorAtPosition<float, 3>& F);
    void integrate_force(
        const VectorAtPosition<float, 3>& F,
        const FixedArray<float, 3>& n,
        float damping,
        float friction);
    void integrate_gravity(const FixedArray<float, 3>& g);
    void advance_time(
        float dt,
        float min_acceleration,
        float min_velocity,
        float min_angular_velocity,
        PhysicsType physics_type,
        ResolveCollisionType resolve_collision_type,
        float hand_brake_velocity,
        std::list<Beacon>* beacons);
    float mass() const;
    FixedArray<float, 3> abs_com() const;
    FixedArray<float, 3, 3> abs_I() const;
    VectorAtPosition<float, 3> abs_F(const VectorAtPosition<float, 3>& F) const;
    FixedArray<float, 3> velocity_at_position(const FixedArray<float, 3>& position) const;
    void set_max_velocity(float max_velocity);
    void set_tire_angle_y(size_t id, float angle_y);
    // void set_tire_accel_x(size_t id, float accel_x);
    FixedArray<float, 3, 3> get_abs_tire_rotation_matrix(size_t id) const;
    FixedArray<float, 3> get_abs_tire_z(size_t id) const;
    float get_tire_angular_velocity(size_t id) const;
    void set_tire_angular_velocity(size_t id, float w);
    FixedArray<float, 3> get_velocity_at_tire_contact(
        const FixedArray<float, 3>& surface_normal,
        size_t id) const;
    float get_angular_velocity_at_tire(
        const FixedArray<float, 3>& surface_normal,
        const FixedArray<float, 3>& street_velocity,
        size_t id) const;
    float get_tire_radius(size_t id) const;
    PowerIntent consume_tire_surface_power(size_t id);
    void set_surface_power(const std::string& engine_name, float surface_power);
    float get_tire_break_force(size_t id) const;
    TrackingWheel& get_tire_tracking_wheel(size_t id);
    FixedArray<float, 3> get_abs_tire_contact_position(size_t id) const;
    const Tire& get_tire(size_t id) const;
    Tire& get_tire(size_t id);
    // void set_tire_sliding(size_t id, bool value);
    // bool get_tire_sliding(size_t id) const;
    float energy() const;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, 3> get_new_absolute_model_matrix() const override;
    virtual void notify_destroyed(void* obj) override;
    virtual void write_status(std::ostream& ostr, StatusComponents log_components) const override;

    RigidBodies& rigid_bodies_;

    float max_velocity_;
#ifdef COMPUTE_POWER
    float power_;
    float energy_old_;
#endif
    std::map<size_t, Tire> tires_;
    std::map<std::string, RigidBodyEngine> engines_;
    // std::map<size_t, bool> tire_sliding_;
    FixedArray<float, 3> tires_z_;

    RigidBodyIntegrator rbi_;
    std::list<std::shared_ptr<CollisionObserver>> collision_observers_;

    Damageable* damageable_;

    Player* driver_;

    const TransformationMatrix<double, 3>* geographic_mapping_;

    mutable std::mutex advance_time_mutex_;
};

}
