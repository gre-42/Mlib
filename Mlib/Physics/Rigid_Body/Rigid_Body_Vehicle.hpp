#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Memory/Destruction_Observers.hpp>
#include <Mlib/Object.hpp>
#include <Mlib/Physics/Actuators/Rigid_Body_Engine.hpp>
#include <Mlib/Physics/Actuators/Tire.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>
#include <Mlib/Physics/Misc/Inventory.hpp>
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
class AnimationStateUpdater;
class RigidBodyAvatarController;
class RigidBodyPlaneController;
class RigidBodyVehicleController;
struct BaseRotor;
class ContactInfo;
class Wing;
class Rotor;

enum class TireAngularVelocityChange {
    OFF,
    IDLE,
    ACCELERATE,
    BRAKE
};

struct JumpState {
    bool wants_to_jump_;
    bool wants_to_jump_oversampled_;
    size_t jumping_counter_;
};

struct GrindState {
    std::optional<FixedArray<float, 3>> grind_point_;
    bool wants_to_grind_;
    size_t wants_to_grind_counter_;
    FixedArray<float, 3> grind_direction_;
    bool grinding_;
    size_t grind_axis_;
    FixedArray<float, 3> grind_pv_;
};

struct AlignToSurfaceState {
    float align_to_surface_relaxation_;
    bool touches_alignment_plane_;
    FixedArray<float, 3> surface_normal_;
};

struct RevertSurfacePowerState {
    float revert_surface_power_threshold_;
    bool revert_surface_power_;
};

struct FlyForwardState {
    float wants_to_fly_forward_factor_;
};

/**
 * From: https://en.wikipedia.org/wiki/Torque#Definition_and_relation_to_angular_momentum
 */
class RigidBodyVehicle: public Object, public DestructionObserver, public AbsoluteMovable, public StatusWriter {
public:
    RigidBodyVehicle(
        const RigidBodyIntegrator& rbi,
        const std::string& name,
        const TransformationMatrix<double, double, 3>* geographic_mapping = nullptr);
    RigidBodyVehicle(const RigidBodyVehicle&) = delete;
    RigidBodyVehicle& operator = (const RigidBodyVehicle&) = delete;
    ~RigidBodyVehicle();
    void reset_forces(size_t oversampling_iteration);
    void integrate_force(
        const VectorAtPosition<float, double, 3>& F,
        const PhysicsEngineConfig& cfg);
    void integrate_force(
        const VectorAtPosition<float, double, 3>& F,
        const FixedArray<float, 3>& n,
        float damping,
        float friction,
        const PhysicsEngineConfig& cfg);
    void integrate_gravity(const FixedArray<float, 3>& g);
    void collide_with_air(
        const PhysicsEngineConfig& cfg,
        std::list<std::unique_ptr<ContactInfo>>& contact_infos);
    void advance_time(
        const PhysicsEngineConfig& cfg,
        std::list<Beacon>* beacons);
    float mass() const;
    FixedArray<double, 3> abs_com() const;
    FixedArray<float, 3, 3> abs_I() const;
    FixedArray<double, 3> abs_grind_point() const;
    FixedArray<double, 3> abs_target() const;
    VectorAtPosition<float, double, 3> abs_F(const VectorAtPosition<float, double, 3>& F) const;
    FixedArray<float, 3> velocity_at_position(const FixedArray<double, 3>& position) const;
    void set_max_velocity(float max_velocity);
    void set_tire_angle_y(size_t id, float angle_y);
    // void set_tire_accel_x(size_t id, float accel_x);
    void set_rotor_angle_x(size_t id, float angle_x);
    void set_rotor_angle_y(size_t id, float angle_y);
    void set_rotor_angle_z(size_t id, float angle_z);
    void set_rotor_movement_x(size_t id, float movement_x);
    void set_rotor_movement_y(size_t id, float movement_y);
    void set_rotor_movement_z(size_t id, float movement_z);
    void set_wing_angle_of_attack(size_t id, float angle);
    void set_wing_brake_angle(size_t id, float angle);
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
    TirePowerIntent consume_tire_surface_power(size_t id);
    TirePowerIntent consume_rotor_surface_power(size_t id);
    void set_surface_power(
        const std::string& engine_name,
        const EnginePowerIntent& engine_power_intent);
    float get_tire_break_force(size_t id) const;
    FixedArray<double, 3> get_abs_tire_contact_position(size_t id) const;
    const Tire& get_tire(size_t id) const;
    Tire& get_tire(size_t id);
    const Rotor& get_rotor(size_t id) const;
    Rotor& get_rotor(size_t id);
    const Wing& get_wing(size_t id) const;
    Wing& get_wing(size_t id);
    // void set_tire_sliding(size_t id, bool value);
    // bool get_tire_sliding(size_t id) const;
    float energy() const;
    const std::string& name() const;
    void set_rigid_bodies(RigidBodies& rigid_bodies);
    void set_wants_to_jump();
    void set_jump_strength(float value);

    // AbsoluteMovable
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, double, 3> get_new_absolute_model_matrix() const override;

    // DestructionObserver
    virtual void notify_destroyed(Object* obj) override;

    // StatusWriter
    virtual void write_status(std::ostream& ostr, StatusComponents log_components) const override;
    RigidBodyAvatarController& avatar_controller();
    RigidBodyPlaneController& plane_controller();
    RigidBodyVehicleController& vehicle_controller();

    DestructionObservers destruction_observers;

    RigidBodies* rigid_bodies_;

    float max_velocity_;
#ifdef COMPUTE_POWER
    float power_;
    float energy_old_;
#endif
    std::map<size_t, Tire> tires_;
    std::map<size_t, std::unique_ptr<Rotor>> rotors_;
    std::map<size_t, std::unique_ptr<Wing>> wings_;
    std::map<std::string, RigidBodyEngine> engines_;
    bool feels_gravity_;
    Inventory inventory_;
    // std::map<size_t, bool> tire_sliding_;
    FixedArray<float, 3> tires_z_;

    // The relative offset when this object is targeted.
    FixedArray<float, 3> target_;

    RigidBodyIntegrator rbi_;
    std::list<std::shared_ptr<CollisionObserver>> collision_observers_;

    std::string name_;
    Damageable* damageable_;
    AnimationStateUpdater* animation_state_updater_;
    IPlayer* driver_;
    std::unique_ptr<RigidBodyAvatarController> avatar_controller_;
    std::unique_ptr<RigidBodyPlaneController> plane_controller_;
    std::unique_ptr<RigidBodyVehicleController> vehicle_controller_;
    float jump_strength_ = 0.25f;
    JumpState jump_state_;
    GrindState grind_state_;
    AlignToSurfaceState align_to_surface_state_;
    RevertSurfacePowerState revert_surface_power_state_;
    FlyForwardState fly_forward_state_;
    const TransformationMatrix<double, double, 3>* geographic_mapping_;
private:
    void advance_time_skate(const PhysicsEngineConfig& cfg);
};

}
