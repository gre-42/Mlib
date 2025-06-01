#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Interfaces/ISpawner.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <string>

namespace Mlib {

class Scene;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
struct GeometrySpawnArguments;
struct NodeSpawnArguments;
class SceneVehicle;
class RigidBodyVehicle;
class Player;

enum class SpawnVehicleAlreadySetBehavior {
    THROW,
    UPDATE
};

SpawnVehicleAlreadySetBehavior spawn_vehicle_already_set_behavior_from_string(const std::string& s);

class VehicleSpawner final : public ISpawner, public virtual DanglingBaseClass {
    VehicleSpawner(const VehicleSpawner&) = delete;
    VehicleSpawner& operator = (const VehicleSpawner&) = delete;
public:
    using TrySpawnVehicle = std::function<bool(
        const TransformationMatrix<SceneDir, CompressedScenePos, 3>& spawn_point,
        const GeometrySpawnArguments& geom,
        const NodeSpawnArguments* node)>;

    VehicleSpawner(
        Scene& scene,
        std::string suffix,
        std::string team_name);
    ~VehicleSpawner();

    // ISpawner
    virtual DanglingBaseClassPtr<IPlayer> player() override;

    // Misc
    float get_respawn_cooldown_time() const;
    void set_respawn_cooldown_time(float respawn_cooldown_time);
    float get_time_since_deletion() const;

    void set_team_name(const std::string& team_name);
    std::string get_team_name() const;
    
    bool has_player() const;
    void set_player(
        const DanglingBaseClassRef<Player>& player,
        std::string role);
    DanglingBaseClassRef<Player> get_player();
    
    bool has_scene_vehicle() const;
    DanglingBaseClassRef<SceneVehicle> get_primary_scene_vehicle();
    DanglingBaseClassRef<const SceneVehicle> get_primary_scene_vehicle() const;
    const std::list<std::unique_ptr<SceneVehicle, DeleteFromPool<SceneVehicle>>>& get_scene_vehicles() const;
    void set_scene_vehicles(std::list<std::unique_ptr<SceneVehicle, DeleteFromPool<SceneVehicle>>>&& scene_vehicle);
    
    void set_spawn_vehicle(
        TrySpawnVehicle try_spawn_vehicle,
        SpawnVehicleAlreadySetBehavior vehicle_spawner_already_set_behavior);
    bool try_spawn(
        const TransformationMatrix<SceneDir, CompressedScenePos, 3>& spawn_point,
        const GeometrySpawnArguments& geometry);
    void delete_vehicle();

    float get_time_since_spawn() const;
    bool get_spotted_by_vip() const;
    void set_spotted_by_vip();

    void advance_time(float dt);
private:
    void notify_spawn();
    Scene& scene_;
    TrySpawnVehicle try_spawn_vehicle_;
    std::list<std::unique_ptr<SceneVehicle, DeleteFromPool<SceneVehicle>>> scene_vehicles_;
    std::list<DestructionFunctionsRemovalTokens> on_scene_vehicle_destroyed_;
    DanglingBaseClassPtr<Player> player_;
    std::string role_;
    DestructionFunctionsRemovalTokens on_player_destroy_;
    std::string suffix_;
    std::string team_name_;
    float time_since_spawn_;
    float time_since_deletion_;
    bool spotted_by_vip_;
    float respawn_cooldown_time_;
};

}
