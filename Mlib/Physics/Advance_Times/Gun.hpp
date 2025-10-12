#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Guards.hpp>
#include <Mlib/Object.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Physics/Misc/Inventory_Item.hpp>
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
struct BulletExplosion;
struct BulletProperties;
enum class RigidBodyVehicleFlags;
struct StaticWorld;
template <class TPosition>
struct AudioSourceState;

using UpdateAudioSourceState = std::function<void(const AudioSourceState<ScenePos>*)>;

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
        const DanglingBaseClassRef<SceneNode>& node,
        const DanglingBaseClassPtr<SceneNode>& punch_angle_node,
        const BulletProperties& bullet_properties,
        std::function<void(
            const std::optional<std::string>& player,
            const std::string& bullet_suffix,
            const std::optional<VariableAndHash<std::string>>& target,
            const FixedArray<float, 3>& velocity,
            const FixedArray<float, 3>& angular_velocity)> generate_smart_bullet,
        std::function<void(const AudioSourceState<ScenePos>&)> generate_shot_audio,
        std::function<void(const AudioSourceState<ScenePos>&, const BulletExplosion&)> generate_bullet_explosion_audio,
        std::function<UpdateAudioSourceState(const AudioSourceState<ScenePos>&)> generate_bullet_engine_audio,
        ITrailStorage* bullet_trace_storage,
        std::string ammo_type,
        std::function<FixedArray<float, 3>(bool shooting)> punch_angle_rng,
        std::function<void(const StaticWorld&)> generate_muzzle_flash);
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
    void set_ypln_node(const DanglingBaseClassRef<SceneNode>& node);
    DanglingBaseClassPtr<SceneNode> get_ypln_node() const;
private:
    bool maybe_generate_bullet(const StaticWorld& world);
    void generate_bullet(const StaticWorld& world);
    void generate_muzzle_flash(const StaticWorld& world);
    void generate_shot_audio();
    RenderingResources* rendering_resources_;
    Scene& scene_;
    SceneNodeResources& scene_node_resources_;
    SmokeParticleGenerator& smoke_generator_;
    DynamicLights& dynamic_lights_;
    RigidBodies& rigid_bodies_;
    AdvanceTimes& advance_times_;
    RigidBodyVehicle& parent_rb_;
    DanglingBaseClassPtr<SceneNode> node_;
    DanglingBaseClassPtr<SceneNode> punch_angle_node_;
    DanglingBaseClassPtr<SceneNode> ypln_node_;
    const BulletProperties& bullet_properties_;
    std::function<void(
        const std::optional<std::string>& player,
        const std::string& bullet_suffix,
        const std::optional<VariableAndHash<std::string>>& target,
        const FixedArray<float, 3>& velocity,
        const FixedArray<float, 3>& angular_velocity)> generate_smart_bullet_;
    std::function<void(const AudioSourceState<ScenePos>&)> generate_shot_audio_;
    std::function<void(const AudioSourceState<ScenePos>&, const BulletExplosion&)> generate_bullet_explosion_audio_;
    std::function<UpdateAudioSourceState(const AudioSourceState<ScenePos>&)> generate_bullet_engine_audio_;
    ITrailStorage* bullet_trace_storage_;
    InventoryItem ammo_type_;
    bool triggered_;
    IPlayer* player_;
    ITeam* team_;
    float cool_down_;
    float time_since_last_shot_;
    TransformationMatrix<float, ScenePos, 3> absolute_model_matrix_;
    FixedArray<float, 3> punch_angle_;
    std::function<FixedArray<float, 3>(bool shooting)> punch_angle_rng_;
    std::function<void(const StaticWorld&)> generate_muzzle_flash_;
    DestructionGuards dgs_;
    DestructionFunctionsRemovalTokens node_on_clear_;
    std::optional<DestructionFunctionsRemovalTokens> punch_angle_node_on_clear_;
    std::optional<DestructionFunctionsRemovalTokens> ypln_node_on_clear_;
};

}
