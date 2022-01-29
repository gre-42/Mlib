#include "Vehicle_Changer.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>

using namespace Mlib;

VehicleChanger::VehicleChanger(
    Players& players,
    DeleteNodeMutex& delete_node_mutex)
: players_{ players },
  delete_node_mutex_{ delete_node_mutex }
{}

void VehicleChanger::change_vehicles() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    for (const auto& [_, p] : players_.players()) {
        auto next_rb = p->next_vehicle().rb;
        if ((next_rb != nullptr) &&
            (!p->has_rigid_body() || (next_rb != &p->rigid_body())))
        {
            if (p->has_rigid_body()) {
                if (next_rb->driver_ != nullptr) {
                    Player* driver = dynamic_cast<Player*>(next_rb->driver_);
                    if (driver == nullptr) {
                        throw std::runtime_error("Next vehicle's driver is not a player");
                    }
                    driver->reset_node();
                }
                p->reset_node();
                p->set_rigid_body(p->next_vehicle());
            }
        }
    }
}
