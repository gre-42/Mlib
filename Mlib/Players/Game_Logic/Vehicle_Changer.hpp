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
private:
    void change_vehicles();
    void swap_vehicles(Player& a, Player& b);
    void enter_vehicle(VehicleSpawner& a, SceneVehicle& b);
    VehicleSpawners& vehicle_spawners_;
    DeleteNodeMutex& delete_node_mutex_;
};

}
