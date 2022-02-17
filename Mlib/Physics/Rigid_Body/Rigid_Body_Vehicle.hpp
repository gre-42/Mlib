#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Actuators/Rotor.hpp>
#include <Mlib/Physics/Actuators/Tire.hpp>
#include <Mlib/Physics/Collision/Resolve_Collision_Type.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Integrator.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Movable.hpp>
#include <map>
#include <memory>
#include <mutex>
#include <optional>

namespace Mlib {

class RigidBodies;
class RigidBodyEngine;
struct Beacon;
class Damageable;
class IPlayer;
class StyleUpdater;
class RigidBodyVehicleController;
class RigidBodyAvatarController;
struct BaseRotor;
class ContactInfo;

enum class TireAngularVelocityChange {
    OFF,
    IDLE,
    ACCELERATE,
    BREAK
};

/**
 * From: https://en.wikipedia.org/wiki/Torque#Definition_and_relation_to_angular_momentum
 */
class RigidBodyVehicle: public DestructionObserver, public AbsoluteMovable, public StatusWriter {
public:
    RigidBodyVehicle(
        RigidBodies& rigid_bodies,
        const RigidBodyIntegrator& rbi,
        const TransformationMatrix<double, 3>* geographic_mapping = nullptr,
        const std::string& name = "");
    ~RigidBodyVehicle();
    void reset_forces();
    void integrate_force(
        const VectorAtPosition<float, 3>& F,
        const PhysicsEngineConfig& cfg);
    void integrate_force(
        const VectorAtPosition<float, 3>& F,
        const FixedArray<float, 3>& n,
        float damping,
        float friction,
        const PhysicsEngineConfig& cfg);
    void integrate_gravity(const FixedArray<float, 3>& g);
    void collide_with_air(
        const PhysicsEngineConfig& cfg,
        std::list<std::unique_ptr<ContactInfo>>& contact_infos);
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
    FixedArray<float, 3> abs_grind_point() const;
    FixedArray<float, 3> abs_target() const;
    VectorAtPosition<float, 3> abs_F(const VectorAtPosition<float, 3>& F) const;
    FixedArray<float, 3> velocity_at_position(const FixedArray<float, 3>& position) const;
    void set_max_velocity(float max_velocity);
    void set_tire_angle_y(size_t id, float angle_y);
    // void set_tire_accel_x(size_t id, float accel_x);
    void set_rotor_angle_x(size_t id, float angle_x);
    void set_rotor_angle_y(size_t id, float angle_y);
    void set_rotor_angle_z(size_t id, float angle_z);
    void set_rotor_movement_x(size_t id, float movement_x);
    void set_rotor_movement_y(size_t id, float movement_y);
    void set_rotor_movement_z(size_t id, float movement_z);
    FixedArray<float, 3, 3> get_abs_tire_rotation_matrix(size_t id) const;
    FixedArray<float, 3> get_abs_tire_z(size_t id) const;
    float get_tire_angular_velocity(size_t id) const;
    void set_tire_angular_velocity(size_t id, float w, TireAngularVelocityChange ch);
    void set_base_angular_velocity(BaseRotor& base_rotor, float w, TireAngularVelocityChange ch);
    FixedArray<float, 3> get_velocity_at_tire_contact(
        const FixedArray<float, 3>& surface_normal,
        size_t id) const;
    float get_angular_velocity_at_tire(
        const FixedArray<float, 3>& surface_normal,
        const FixedArray<float, 3>& street_velocity,
        size_t id) const;
    float get_tire_radius(size_t id) const;
    PowerIntent consume_tire_surface_power(size_t id);
    PowerIntent consume_rotor_surface_power(size_t id);
    void set_surface_power(
        const std::string& engine_name,
        float surface_power,
        float delta_power = 0.f);
    float get_tire_break_force(size_t id) const;
    TrackingWheel& get_tire_tracking_wheel(size_t id);
    FixedArray<float, 3> get_abs_tire_contact_position(size_t id) const;
    const Tire& get_tire(size_t id) const;
    Tire& get_tire(size_t id);
    const Rotor& get_rotor(size_t id) const;
    Rotor& get_rotor(size_t id);
    // void set_tire_sliding(size_t id, bool value);
    // bool get_tire_sliding(size_t id) const;
    float energy() const;
    const std::string& name() const;

    // AbsoluteMovable
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, 3> get_new_absolute_model_matrix() const override;

    // DestructionObserver
    virtual void notify_destroyed(void* obj) override;

    // StatusWriter
    virtual void write_status(std::ostream& ostr, StatusComponents log_components) const override;
    RigidBodyAvatarController& avatar_controller();
    RigidBodyVehicleController& vehicle_controller();

    RigidBodies& rigid_bodies_;

    float max_velocity_;
#ifdef COMPUTE_POWER
    float power_;
    float energy_old_;
#endif
    std::map<size_t, Tire> tires_;
    std::map<size_t, std::unique_ptr<Rotor>> rotors_;
    std::map<std::string, RigidBodyEngine> engines_;
    // std::map<size_t, bool> tire_sliding_;
    FixedArray<float, 3> tires_z_;

    std::optional<FixedArray<float, 3>> grind_point_;

    // The relative offset when this object is targeted.
    FixedArray<float, 3> target_;

    RigidBodyIntegrator rbi_;
    std::list<std::shared_ptr<CollisionObserver>> collision_observers_;

    std::string name_;
    Damageable* damageable_;
    StyleUpdater* style_updater_;
    IPlayer* driver_;
    std::unique_ptr<RigidBodyAvatarController> avatar_controller_;
    std::unique_ptr<RigidBodyVehicleController> vehicle_controller_;
    float align_to_surface_relaxation_;
    bool wants_to_jump_;
    bool wants_to_grind_;
    bool grinding_;
    FixedArray<float, 3> surface_normal_;
    float revert_surface_power_threshold_;
    bool revert_surface_power_;
    const TransformationMatrix<double, 3>* geographic_mapping_;
    mutable std::mutex advance_time_mutex_;
};

}
