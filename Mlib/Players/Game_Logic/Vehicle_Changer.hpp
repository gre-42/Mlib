#pragma once

namespace Mlib {

class SceneVehicle;
class Player;
class Players;
class GameLogic;
class DeleteNodeMutex;

class VehicleChanger {
    friend GameLogic;
public:
    VehicleChanger(
        Players& players,
        DeleteNodeMutex& delete_node_mutex);
private:
    void change_vehicles();
    void swap_vehicles(Player& a, Player& b);
    void enter_vehicle(Player& a, SceneVehicle& b);
    Players& players_;
    DeleteNodeMutex& delete_node_mutex_;
};

}
