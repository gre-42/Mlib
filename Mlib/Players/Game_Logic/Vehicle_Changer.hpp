#pragma once
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>

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
    bool change_vehicle(VehicleSpawner& s, const SceneTime& time);
private:
    void change_vehicles(const SceneTime& time);
    void swap_vehicles(Player& a, Player& b);
    bool enter_vehicle(VehicleSpawner& a, VehicleSpawner& b, const SceneTime& time);
    VehicleSpawners& vehicle_spawners_;
    SafeAtomicRecursiveSharedMutex& delete_node_mutex_;
};

}
