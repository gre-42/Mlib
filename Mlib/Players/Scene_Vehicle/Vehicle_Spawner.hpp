#pragma once
#include <Mlib/Physics/Interfaces/ISpawner.hpp>
#include <chrono>
#include <functional>
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
    explicit VehicleSpawner(Scene& scene);
    ~VehicleSpawner();

    // ISpawner
    virtual void notify_vehicle_destroyed() override;
    virtual IPlayer* player() override;

    // Misc
    void set_team_name(const std::string& team_name);
    std::string get_team_name() const;
    
    bool has_player() const;
    void set_player(Player& player);
    Player& get_player();
    
    bool has_scene_vehicle() const;
    void clear_scene_vehicle();
    SceneVehicle& get_scene_vehicle();
    void set_scene_vehicle(std::unique_ptr<SceneVehicle>&& scene_vehicle);
    
    void set_spawn_vehicle(std::function<void(const SpawnPoint&)> spawn_vehicle);
    void spawn(const SpawnPoint& spawn_point, double spawn_y_offset);

    float seconds_since_spawn() const;
    bool spotted_by_vip() const;
    void set_spotted_by_vip();
private:
    void notify_spawn();
    Scene& scene_;
    std::function<void(const SpawnPoint&)> spawn_vehicle_;
    std::unique_ptr<SceneVehicle> scene_vehicle_;
    Player* player_;
    std::string team_name_;
    std::chrono::time_point<std::chrono::steady_clock> spawn_time_;
    bool spotted_by_vip_;
};

}