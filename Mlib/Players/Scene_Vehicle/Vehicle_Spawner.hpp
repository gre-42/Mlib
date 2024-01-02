#pragma once
#include <Mlib/Physics/Interfaces/ISpawner.hpp>
#include <chrono>
#include <functional>
#include <list>
#include <memory>
#include <string>

namespace Mlib {

class Scene;
struct SpawnPoint;
class SceneVehicle;
class Player;

class VehicleSpawner final : public ISpawner {
    VehicleSpawner(const VehicleSpawner&) = delete;
    VehicleSpawner& operator = (const VehicleSpawner&) = delete;
public:
    VehicleSpawner(Scene& scene, const std::string& team_name);
    ~VehicleSpawner();

    // ISpawner
    virtual void notify_vehicle_destroyed(RigidBodyVehicle& rigid_body_vehicle) override;
    virtual IPlayer* player() override;

    // Misc
    void set_team_name(const std::string& team_name);
    std::string get_team_name() const;
    
    bool has_player() const;
    void set_player(Player& player);
    Player& get_player();
    
    bool has_scene_vehicle() const;
    SceneVehicle& get_primary_scene_vehicle();
    const std::list<std::unique_ptr<SceneVehicle>>& get_scene_vehicles();
    void set_scene_vehicles(std::list<std::unique_ptr<SceneVehicle>>&& scene_vehicle);
    
    void set_spawn_vehicle(std::function<void(const SpawnPoint&)> spawn_vehicle);
    void spawn(const SpawnPoint& spawn_point, double spawn_y_offset);

    float seconds_since_spawn() const;
    bool spotted_by_vip() const;
    void set_spotted_by_vip();
private:
    void notify_spawn();
    Scene& scene_;
    std::function<void(const SpawnPoint&)> spawn_vehicle_;
    std::list<std::unique_ptr<SceneVehicle>> scene_vehicles_;
    Player* player_;
    std::string team_name_;
    std::chrono::steady_clock::time_point spawn_time_;
    bool spotted_by_vip_;
};

}
