#pragma once

namespace Mlib {

class SceneVehicle;
class VehicleSpawner;
class VehicleSpawners;
class Player;
class GameLogic;
class DeleteNodeMutex;

class VehicleChanger {
    friend GameLogic;
public:
    VehicleChanger(
        VehicleSpawners& vehicle_spawners,
        DeleteNodeMutex& delete_node_mutex);
    bool change_vehicle(VehicleSpawner& s);
private:
    void change_vehicles();
    void swap_vehicles(Player& a, Player& b);
    bool enter_vehicle(VehicleSpawner& a, VehicleSpawner& b);
    VehicleSpawners& vehicle_spawners_;
    DeleteNodeMutex& delete_node_mutex_;
};

}
