#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Guards.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Observer.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <functional>
#include <mutex>
#include <optional>

namespace Mlib {

class RenderingResources;
class SceneNodeResources;
class SmokeParticleGenerator;
class DynamicLights;
class ITrailStorage;
class RigidBodyVehicle;
class Scene;
class RigidBodies;
class AdvanceTimes;
class DeleteNodeMutex;
class SceneNode;
class IPlayer;
class ITeam;
struct BulletProperties;
enum class RigidBodyVehicleFlags;

class Gun final: public IAbsoluteObserver, public IAdvanceTime {
public:
    Gun(RenderingResources* rendering_resources,
        Scene& scene,
        SceneNodeResources& scene_node_resources,
        SmokeParticleGenerator& smoke_generator,
        DynamicLights& dynamic_lights,
        RigidBodies& rigid_bodies,
        AdvanceTimes& advance_times,
        float cool_down,
        RigidBodyVehicle& parent_rb,
        DanglingRef<SceneNode> node,
        DanglingPtr<SceneNode> punch_angle_node,
        const BulletProperties& bullet_properties,
        std::function<void(
            const std::optional<std::string>& player,
            const std::string& bullet_suffix,
            const std::optional<std::string>& target,
            const FixedArray<float, 3>& velocity)> generate_smart_bullet,
        ITrailStorage* bullet_trace_storage,
        const std::string& ammo_type,
        const std::function<FixedArray<float, 3>(bool shooting)>& punch_angle_rng,
        const std::string& muzzle_flash_resource,
        const FixedArray<float, 3>& muzzle_flash_position,
        float muzzle_flash_animation_time,
        const std::function<void(const std::string& muzzle_flash_suffix)>& generate_muzzle_flash_hider,
        DeleteNodeMutex& delete_node_mutex);
    ~Gun();
    virtual void advance_time(float dt, std::chrono::steady_clock::time_point time) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, double, 3>& absolute_model_matrix) override;
    void trigger(
        IPlayer* player = nullptr,
        ITeam* team = nullptr);
    const TransformationMatrix<float, double, 3>& absolute_model_matrix() const;
    bool is_none_gun() const;
    const FixedArray<float, 3>& punch_angle() const;
    size_t nbullets_available() const;
    float cool_down() const;
    float bullet_damage() const;
private:
    bool maybe_generate_bullet(std::chrono::steady_clock::time_point time);
    void generate_bullet(std::chrono::steady_clock::time_point time);
    void generate_muzzle_flash_hider();
    RenderingResources* rendering_resources_;
    Scene& scene_;
    SceneNodeResources& scene_node_resources_;
    SmokeParticleGenerator& smoke_generator_;
    DynamicLights& dynamic_lights_;
    RigidBodies& rigid_bodies_;
    AdvanceTimes& advance_times_;
    RigidBodyVehicle& parent_rb_;
    DanglingPtr<SceneNode> node_;
    DanglingPtr<SceneNode> punch_angle_node_;
    const BulletProperties& bullet_properties_;
    std::function<void(
        const std::optional<std::string>& player,
        const std::string& bullet_suffix,
        const std::optional<std::string>& target,
        const FixedArray<float, 3>& velocity)> generate_smart_bullet_;
    ITrailStorage* bullet_trace_storage_;
    std::string ammo_type_;
    bool triggered_;
    IPlayer* player_;
    ITeam* team_;
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
    DestructionGuards dgs_;
    DestructionFunctionsRemovalTokens node_on_clear_;
    std::optional<DestructionFunctionsRemovalTokens> punch_angle_node_on_clear_;
};

}
