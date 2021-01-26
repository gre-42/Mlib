#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Scene_Graph/Transformation/Absolute_Observer.hpp>
#include <atomic>

namespace Mlib {

class SceneNodeResources;
struct RigidBodyIntegrator;
class Scene;
class RigidBodies;
class AdvanceTimes;

class Gun: public DestructionObserver, public AbsoluteObserver, public AdvanceTime {
public:
    Gun(Scene& scene,
        SceneNodeResources& scene_node_resources,
        RigidBodies& rigid_bodies,
        AdvanceTimes& advance_times,
        float cool_down,
        const RigidBodyIntegrator& parent_rbi,
        const std::string& bullet_renderable_resource_name,
        const std::string& bullet_hitbox_resource_name,
        float bullet_mass,
        float bullet_velocity,
        float bullet_lifetime,
        float bullet_damage,
        const FixedArray<float, 3>& bullet_size);
    virtual void advance_time(float dt) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, 3>& absolute_model_matrix) override;
    virtual void notify_destroyed(void* obj) override;
    void trigger();
private:
    Scene& scene_;
    SceneNodeResources& scene_node_resources_;
    RigidBodies& rigid_bodies_;
    AdvanceTimes& advance_times_;
    const RigidBodyIntegrator& parent_rbi_;
    std::string bullet_renderable_resource_name_;
    std::string bullet_hitbox_resource_name_;
    float bullet_mass_;
    float bullet_velocity_;
    float bullet_lifetime_;
    float bullet_damage_;
    const FixedArray<float, 3> bullet_size_;
    std::atomic_bool triggered_;
    float cool_down_;
    float seconds_since_last_shot_;
    TransformationMatrix<float, 3> absolute_model_matrix_;
};

}
