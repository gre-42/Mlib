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

class VehicleChanger {
    friend GameLogic;
public:
    VehicleChanger(
        VehicleSpawners& vehicle_spawners,
        SafeAtomicRecursiveSharedMutex& delete_node_mutex);
    bool change_vehicle(VehicleSpawner& s, std::chrono::steady_clock::time_point time);
private:
    void change_vehicles(std::chrono::steady_clock::time_point time);
    void swap_vehicles(Player& a, Player& b);
    bool enter_vehicle(VehicleSpawner& a, VehicleSpawner& b, std::chrono::steady_clock::time_point time);
    VehicleSpawners& vehicle_spawners_;
    SafeAtomicRecursiveSharedMutex& delete_node_mutex_;
};

}
