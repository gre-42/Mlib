#pragma once
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <chrono>

namespace Mlib {

class SceneVehicle;
class VehicleSpawner;
class VehicleSpawners;
class Player;
class GameLogic;
class SceneTime;
struct PhysicsEngineConfig;

class VehicleChanger {
    friend GameLogic;
public:
    VehicleChanger(
        VehicleSpawners& vehicle_spawners,
        SafeAtomicRecursiveSharedMutex& delete_node_mutex);
    bool change_vehicle(
        VehicleSpawner& s,
        const PhysicsEngineConfig& cfg,
        std::chrono::steady_clock::time_point time);
private:
    void change_vehicles(
        const PhysicsEngineConfig& cfg,
        std::chrono::steady_clock::time_point time);
    void swap_vehicles(Player& a, Player& b);
    bool enter_vehicle(
        VehicleSpawner& a,
        VehicleSpawner& b,
        const PhysicsEngineConfig& cfg,
        std::chrono::steady_clock::time_point time);
    VehicleSpawners& vehicle_spawners_;
    SafeAtomicRecursiveSharedMutex& delete_node_mutex_;
};

}
