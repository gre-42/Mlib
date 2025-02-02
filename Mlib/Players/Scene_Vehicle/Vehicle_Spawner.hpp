#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/ISpawner.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <string>

namespace Mlib {

class Scene;
struct SpawnPoint;
class SceneVehicle;
class RigidBodyVehicle;
class Player;

class VehicleSpawner final : public ISpawner, public DestructionObserver<const RigidBodyVehicle&>, public virtual DanglingBaseClass {
    VehicleSpawner(const VehicleSpawner&) = delete;
    VehicleSpawner& operator = (const VehicleSpawner&) = delete;
public:
    using SpawnVehicle = std::function<void(const SpawnPoint& spawn_point)>;

    VehicleSpawner(Scene& scene, std::string team_name);
    ~VehicleSpawner();

    // DestructionObserver
    virtual void notify_destroyed(const RigidBodyVehicle& rigid_body_vehicle) override;

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
    SceneVehicle& get_primary_scene_vehicle();
    const std::list<std::unique_ptr<SceneVehicle>>& get_scene_vehicles();
    void set_scene_vehicles(std::list<std::unique_ptr<SceneVehicle>>&& scene_vehicle);
    
    void set_spawn_vehicle(SpawnVehicle spawn_vehicle);
    void spawn(const SpawnPoint& spawn_point, ScenePos spawn_y_offset);

    float get_time_since_spawn() const;
    bool get_spotted_by_vip() const;
    void set_spotted_by_vip();

    void advance_time(float dt);
private:
    void notify_spawn();
    Scene& scene_;
    SpawnVehicle spawn_vehicle_;
    std::list<std::unique_ptr<SceneVehicle>> scene_vehicles_;
    DanglingBaseClassPtr<Player> player_;
    std::string role_;
    DestructionFunctionsRemovalTokens on_player_destroy_;
    std::string team_name_;
    float time_since_spawn_;
    float time_since_deletion_;
    bool spotted_by_vip_;
    float respawn_cooldown_time_;
};

}
