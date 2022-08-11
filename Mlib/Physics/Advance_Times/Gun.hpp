#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Observer.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <atomic>
#include <mutex>

namespace Mlib {

class SceneNodeResources;
struct RigidBodyVehicle;
class Scene;
class RigidBodies;
class AdvanceTimes;
class DeleteNodeMutex;
class SceneNode;

class Gun: public DestructionObserver, public AbsoluteObserver, public AdvanceTime {
public:
    Gun(Scene& scene,
        SceneNodeResources& scene_node_resources,
        RigidBodies& rigid_bodies,
        AdvanceTimes& advance_times,
        float cool_down,
        RigidBodyVehicle& parent_rb,
        SceneNode& punch_angle_node,
        const std::string& bullet_renderable_resource_name,
        const std::string& bullet_hitbox_resource_name,
        float bullet_mass,
        float bullet_velocity,
        float bullet_lifetime,
        float bullet_damage,
        const FixedArray<float, 3>& bullet_size,
        const std::string& ammo_type,
        float punch_angle_idle_std,
        float punch_angle_shoot_std,
        DeleteNodeMutex& delete_node_mutex);
    virtual void advance_time(float dt) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) override;
    virtual void notify_destroyed(void* obj) override;
    void trigger();
    const TransformationMatrix<float, double, 3>& absolute_model_matrix() const;
    bool is_none_gun() const;
    const FixedArray<float, 3>& punch_angle() const;
    size_t nbullets_available() const;
private:
    void maybe_generate_bullet();
    void generate_bullet();
    Scene& scene_;
    SceneNodeResources& scene_node_resources_;
    RigidBodies& rigid_bodies_;
    AdvanceTimes& advance_times_;
    RigidBodyVehicle& parent_rb_;
    SceneNode& punch_angle_node_;
    std::string bullet_renderable_resource_name_;
    std::string bullet_hitbox_resource_name_;
    float bullet_mass_;
    float bullet_velocity_;
    float bullet_lifetime_;
    float bullet_damage_;
    const FixedArray<float, 3> bullet_size_;
    std::string ammo_type_;
    std::atomic_bool triggered_;
    float cool_down_;
    float time_since_last_shot_;
    TransformationMatrix<float, double, 3> absolute_model_matrix_;
    DeleteNodeMutex& delete_node_mutex_;
    FixedArray<float, 3> punch_angle_;
    NormalRandomNumberGenerator<float> punch_angle_idle_rng_;
    NormalRandomNumberGenerator<float> punch_angle_shoot_rng_;
};

}
