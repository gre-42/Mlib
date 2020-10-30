#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Misc/Rigid_Body_Integrator.hpp>
#include <Mlib/Physics/Misc/Tire.hpp>
#include <Mlib/Scene_Graph/Loggable.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <map>
#include <memory>
#include <mutex>

namespace Mlib {

class RigidBodies;
class RigidBodyEngine;

/**
 * From: https://en.wikipedia.org/wiki/Torque#Definition_and_relation_to_angular_momentum
 */
class RigidBody: public DestructionObserver, public AbsoluteMovable, public Loggable {
public:
    RigidBody(
        RigidBodies& rigid_bodies,
        float mass,
        const FixedArray<float, 3>& L,    // angular momentum
        const FixedArray<float, 3, 3>& I, // inertia tensor
        const FixedArray<float, 3>& com,  // center of mass
        const FixedArray<float, 3>& v,    // velocity
        const FixedArray<float, 3>& w,    // angular velocity
        const FixedArray<float, 3>& T,    // torque
        const FixedArray<float, 3>& position,
        const FixedArray<float, 3>& rotation,
        bool I_is_diagonal);
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
        std::vector<FixedArray<float, 3>>& beacons);
    float mass() const;
    FixedArray<float, 3> abs_com() const;
    FixedArray<float, 3, 3> abs_I() const;
    VectorAtPosition<float, 3> abs_F(const VectorAtPosition<float, 3>& F) const;
    FixedArray<float, 3> velocity_at_position(const FixedArray<float, 3>& position) const;
    void set_max_velocity(float max_velocity);
    void set_tire_angle(size_t id, float angle);
    FixedArray<float, 3, 3> get_abs_tire_rotation_matrix(size_t id) const;
    FixedArray<float, 3> get_abs_tire_z(size_t id) const;
    float consume_tire_surface_power(size_t id);
    void set_surface_power(const std::string& engine_name, float surface_power);
    float get_tire_break_force(size_t id) const;
    StickyWheel& get_tire_sticky_wheel(size_t id);
    FixedArray<float, 3> get_abs_tire_position(size_t id) const;
    // void set_tire_sliding(size_t id, bool value);
    // bool get_tire_sliding(size_t id) const;
    float energy() const;
    virtual void set_absolute_model_matrix(const FixedArray<float, 4, 4>& absolute_model_matrix) override;
    virtual FixedArray<float, 4, 4> get_new_absolute_model_matrix() const override;
    virtual void notify_destroyed(void* obj) override;
    virtual void log(std::ostream& ostr, unsigned int log_components) const override;

    RigidBodies& rigid_bodies_;

    float max_velocity_;
    std::map<size_t, Tire> tires_;
    std::map<std::string, RigidBodyEngine> engines_;
    // std::map<size_t, bool> tire_sliding_;
    FixedArray<float, 3> tires_z_;

    float mass_;
    RigidBodyIntegrator rbi_;
    std::list<std::shared_ptr<CollisionObserver>> collision_observers_;
    mutable std::mutex advance_time_mutex_;
};

}
