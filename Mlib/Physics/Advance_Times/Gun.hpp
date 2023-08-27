#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Observer.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <mutex>

namespace Mlib {

class SceneNodeResources;
class SmokeParticleGenerator;
class RigidBodyVehicle;
class Scene;
class RigidBodies;
class AdvanceTimes;
class DeleteNodeMutex;
class SceneNode;
class Player;
class Team;
enum class RigidBodyVehicleFlags;

class Gun final: public DestructionObserver<DanglingRef<const SceneNode>>, public AbsoluteObserver, public AdvanceTime {
public:
    Gun(Scene& scene,
        SceneNodeResources& scene_node_resources,
        SmokeParticleGenerator& smoke_generator,
        RigidBodies& rigid_bodies,
        AdvanceTimes& advance_times,
        float cool_down,
        RigidBodyVehicle& parent_rb,
        DanglingRef<SceneNode> node,
        DanglingRef<SceneNode> punch_angle_node,
        const std::string& bullet_renderable_resource_name,
        const std::string& bullet_hitbox_resource_name,
        const std::string& bullet_explosion_resource_name,
        float bullet_explosion_animation_time,
        RigidBodyVehicleFlags bullet_rigid_body_flags,
        float bullet_mass,
        float bullet_velocity,
        float bullet_lifetime,
        float bullet_damage,
        float bullet_damage_radius,
        const FixedArray<float, 3>& bullet_size,
        const std::string& bullet_trail_resource,
        float bullet_trail_dt,
        float bullet_trail_animation_time,
        const std::string& ammo_type,
        const std::function<FixedArray<float, 3>(bool shooting)>& punch_angle_rng,
        const std::string& muzzle_flash_resource,
        const FixedArray<float, 3>& muzzle_flash_position,
        float muzzle_flash_animation_time,
        const std::function<void(const std::string& muzzle_flash_suffix)>& generate_muzzle_flash_hider,
        DeleteNodeMutex& delete_node_mutex);
    ~Gun();
    virtual void advance_time(float dt) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) override;
    virtual void notify_destroyed(DanglingRef<const SceneNode> destroyed_object) override;
    void trigger(
        Player* player = nullptr,
        Team* team = nullptr);
    const TransformationMatrix<float, double, 3>& absolute_model_matrix() const;
    bool is_none_gun() const;
    const FixedArray<float, 3>& punch_angle() const;
    size_t nbullets_available() const;
    float cool_down() const;
    float bullet_damage() const;
private:
    bool maybe_generate_bullet();
    void generate_bullet();
    void generate_muzzle_flash_hider();
    Scene& scene_;
    SceneNodeResources& scene_node_resources_;
    SmokeParticleGenerator& smoke_generator_;
    RigidBodies& rigid_bodies_;
    AdvanceTimes& advance_times_;
    RigidBodyVehicle& parent_rb_;
    DanglingRef<SceneNode> node_;
    DanglingRef<SceneNode> punch_angle_node_;
    std::string bullet_renderable_resource_name_;
    std::string bullet_hitbox_resource_name_;
    std::string bullet_explosion_resource_name_;
    float bullet_explosion_animation_time_;
    RigidBodyVehicleFlags bullet_rigid_body_flags_;
    float bullet_mass_;
    float bullet_velocity_;
    float bullet_lifetime_;
    float bullet_damage_;
    float bullet_damage_radius_;
    const FixedArray<float, 3> bullet_size_;
    std::string bullet_trail_resource_;
    float bullet_trail_dt_;
    float bullet_trail_animation_time_;
    std::string ammo_type_;
    bool triggered_;
    Player* player_;
    Team* team_;
    float cool_down_;
    float time_since_last_shot_;
    TransformationMatrix<float, double, 3> absolute_model_matrix_;
    FixedArray<float, 3> punch_angle_;
    std::function<FixedArray<float, 3>(bool shooting)> punch_angle_rng_;
    std::string muzzle_flash_resource_;
    FixedArray<float, 3> muzzle_flash_position_;
    float muzzle_flash_animation_time_;
    std::function<void(const std::string& muzzle_flash_suffix)> generate_muzzle_flash_hider_;
    DeleteNodeMutex& delete_node_mutex_;
};

}
