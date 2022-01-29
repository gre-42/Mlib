#pragma once

namespace Mlib {

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
    Players& players_;
    DeleteNodeMutex& delete_node_mutex_;
};

}
