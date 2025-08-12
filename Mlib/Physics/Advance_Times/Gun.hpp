#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Guards.hpp>
#include <Mlib/Object.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Observer.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <Mlib/Variable_And_Hash.hpp>
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
class SceneNode;
class IPlayer;
class ITeam;
struct BulletProperties;
enum class RigidBodyVehicleFlags;
struct StaticWorld;

class Gun final: public IAbsoluteObserver, public IAdvanceTime, public virtual DanglingBaseClass {
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
        const DanglingRef<SceneNode>& node,
        const DanglingPtr<SceneNode>& punch_angle_node,
        const BulletProperties& bullet_properties,
        std::function<void(
            const std::optional<std::string>& player,
            const std::string& bullet_suffix,
            const std::optional<VariableAndHash<std::string>>& target,
            const FixedArray<float, 3>& velocity,
            const FixedArray<float, 3>& angular_velocity)> generate_smart_bullet,
        std::function<void(
            const FixedArray<ScenePos, 3>& position,
            const FixedArray<SceneDir, 3>& velocity)> generate_shot_audio,
        ITrailStorage* bullet_trace_storage,
        std::string ammo_type,
        std::function<FixedArray<float, 3>(bool shooting)> punch_angle_rng,
        VariableAndHash<std::string> muzzle_flash_resource,
        const FixedArray<float, 3>& muzzle_flash_position,
        float muzzle_flash_animation_time,
        std::function<void(const std::string& muzzle_flash_suffix)> generate_muzzle_flash_hider);
    ~Gun();
    virtual void advance_time(float dt, const StaticWorld& world) override;
    virtual void set_absolute_model_matrix(const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix) override;
    void trigger(
        IPlayer* player = nullptr,
        ITeam* team = nullptr);
    const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix() const;
    bool is_none_gun() const;
    const FixedArray<float, 3>& punch_angle() const;
    size_t nbullets_available() const;
    float cool_down() const;
    float bullet_damage() const;
private:
    bool maybe_generate_bullet(const StaticWorld& world);
    void generate_bullet(const StaticWorld& world);
    void generate_muzzle_flash_hider();
    void generate_shot_audio();
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
        const std::optional<VariableAndHash<std::string>>& target,
        const FixedArray<float, 3>& velocity,
        const FixedArray<float, 3>& angular_velocity)> generate_smart_bullet_;
    std::function<void(
        const FixedArray<ScenePos, 3>& position,
        const FixedArray<SceneDir, 3>& velocity)> generate_shot_audio_;
    ITrailStorage* bullet_trace_storage_;
    std::string ammo_type_;
    bool triggered_;
    IPlayer* player_;
    ITeam* team_;
    float cool_down_;
    float time_since_last_shot_;
    TransformationMatrix<float, ScenePos, 3> absolute_model_matrix_;
    FixedArray<float, 3> punch_angle_;
    std::function<FixedArray<float, 3>(bool shooting)> punch_angle_rng_;
    VariableAndHash<std::string> muzzle_flash_resource_;
    FixedArray<float, 3> muzzle_flash_position_;
    float muzzle_flash_animation_time_;
    std::function<void(const std::string& muzzle_flash_suffix)> generate_muzzle_flash_hider_;
    DestructionGuards dgs_;
    DestructionFunctionsRemovalTokens node_on_clear_;
    std::optional<DestructionFunctionsRemovalTokens> punch_angle_node_on_clear_;
};

}
