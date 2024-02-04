#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Vector_At_Position.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Memory/Destruction_Observers.hpp>
#include <Mlib/Physics/Containers/Rigid_Bodies.hpp>
#include <Mlib/Physics/Interfaces/Collision_Observer.hpp>
#include <Mlib/Physics/Misc/Inventory.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Pulses.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Movable.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/INode_Hider.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <map>
#include <memory>
#include <optional>
#include <set>

namespace Mlib {

class RigidBodies;
class RigidBodyEngine;
class RigidBodyDeltaEngine;
struct TirePowerIntent;
struct EnginePowerIntent;
struct EnginePowerDeltaIntent;
struct Beacon;
class Damageable;
class ISpawner;
class IPlayer;
class AnimationStateUpdater;
class RigidBodyAvatarController;
class RigidBodyPlaneController;
class RigidBodyVehicleController;
struct BaseRotor;
class Tire;
class Rotor;
class ContactInfo;
class Wing;
enum class VelocityClassification;
enum class RigidBodyVehicleFlags;

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

struct TrailerHitches {
    std::optional<FixedArray<float, 3>> female_;
    std::optional<FixedArray<float, 3>> male_;
    FixedArray<float, 3> get_position_female() const;
    FixedArray<float, 3> get_position_male() const;
    void set_position_female(const FixedArray<float, 3>& position);
    void set_position_male(const FixedArray<float, 3>& position);
};

/**
 * From: https://en.wikipedia.org/wiki/Torque#Definition_and_relation_to_angular_momentum
 */
class RigidBodyVehicle: public DestructionObserver<DanglingRef<const SceneNode>>, public IAbsoluteMovable, public StatusWriter, public INodeHider {
public:
    RigidBodyVehicle(
        const RigidBodyPulses& rbp,
        std::string name,
        std::string asset_id,
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
    FixedArray<float, 3> get_abs_tire_z(size_t id) const;
    float get_tire_angular_velocity(size_t id) const;
    void set_tire_angular_velocity(
        size_t id,
        float w,
        const PhysicsEngineConfig& cfg,
        float& available_power);
    void set_rotor_angular_velocity(
        size_t id,
        float w,
        const PhysicsEngineConfig& cfg,
        float& available_power);
    void set_base_angular_velocity(
        BaseRotor& base_rotor,
        const FixedArray<float, 3>& rotation_axis,
        float w,
        const PhysicsEngineConfig& cfg,
        float& available_power);
    FixedArray<float, 3> get_velocity_at_tire_contact(
        const FixedArray<float, 3>& surface_normal,
        size_t id) const;
    float get_angular_velocity_at_tire(
        const FixedArray<float, 3>& surface_normal,
        const FixedArray<float, 3>& street_velocity,
        size_t id) const;
    float get_tire_radius(size_t id) const;
    TirePowerIntent consume_tire_surface_power(size_t id, VelocityClassification velocity_classification);
    TirePowerIntent consume_rotor_surface_power(size_t id);
    void set_surface_power(
        const std::string& engine_name,
        const EnginePowerIntent& engine_power_intent);
    void set_delta_surface_power(
        const std::string& delta_engine_name,
        const EnginePowerDeltaIntent& engine_power_delta_intent);
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
    // This method is currently just a debugging feature.
    const std::string& asset_id() const;
    void set_rigid_bodies(RigidBodies& rigid_bodies);
    void set_wants_to_jump();
    void set_jump_dv(float value);

    // IAbsoluteMovable
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) override;
    virtual TransformationMatrix<float, double, 3> get_new_absolute_model_matrix() const override;

    // DestructionObserver
    virtual void notify_destroyed(DanglingRef<const SceneNode> destroyed_object) override;

    // StatusWriter
    virtual void write_status(std::ostream& ostr, StatusComponents log_components) const override;
    virtual float get_value(StatusComponents status_components) const override;
    virtual StatusWriter& child_status_writer(const std::vector<std::string>& name) override;

    // INodeHider
    virtual bool node_shall_be_hidden(
        DanglingRef<const SceneNode> camera_node,
        const ExternalRenderPass& external_render_pass) const override;

    bool feels_gravity() const;
    bool is_avatar() const;
    bool is_activated_avatar() const;
    bool is_deactivated_avatar() const;

    RigidBodyAvatarController& avatar_controller();
    RigidBodyPlaneController& plane_controller();
    RigidBodyVehicleController& vehicle_controller();

    DestructionObservers<const RigidBodyVehicle&> destruction_observers;

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
    std::map<std::string, RigidBodyDeltaEngine> delta_engines_;
    RigidBodyVehicleFlags flags_;
    Inventory inventory_;
    // std::map<size_t, bool> tire_sliding_;
    FixedArray<float, 3> tires_z_;

    // The relative offset when this object is targeted.
    FixedArray<float, 3> target_;

    RigidBodyPulses rbp_;
    std::list<std::unique_ptr<CollisionObserver>> collision_observers_;

    std::string name_;
    std::string asset_id_;
    Damageable* damageable_;
    std::set<RigidBodyVehicle*> passengers_;
    float door_distance_;
    AnimationStateUpdater* animation_state_updater_;
    ISpawner* spawner_;
    IPlayer* driver_;
    std::unique_ptr<RigidBodyAvatarController> avatar_controller_;
    std::unique_ptr<RigidBodyPlaneController> plane_controller_;
    std::unique_ptr<RigidBodyVehicleController> vehicle_controller_;
    float jump_dv_ = 17.f * kph;
    JumpState jump_state_;
    GrindState grind_state_;
    AlignToSurfaceState align_to_surface_state_;
    RevertSurfacePowerState revert_surface_power_state_;
    FlyForwardState fly_forward_state_;
    TrailerHitches trailer_hitches_;
    const TransformationMatrix<double, double, 3>* geographic_mapping_;
private:
    void advance_time_skate(const PhysicsEngineConfig& cfg);
};

}
