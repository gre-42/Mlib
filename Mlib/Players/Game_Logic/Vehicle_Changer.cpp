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
                Player* other_driver = nullptr;
                bool other_ec_old;
                PlayerVehicle other_next_vehicle;
                if (next_rb->driver_ != nullptr) {
                    other_driver = dynamic_cast<Player*>(next_rb->driver_);
                    if (other_driver == nullptr) {
                        throw std::runtime_error("Next vehicle's driver is not a player");
                    }
                    other_ec_old = other_driver->externals_created();
                    other_next_vehicle = p->vehicle();
                    other_driver->reset_node();
                }
                {
                    bool ec_old = p->externals_created();
                    p->reset_node();
                    p->set_rigid_body(p->next_vehicle());
                    if (ec_old) {
                        p->create_externals();
                    }
                }
                if (other_driver != nullptr) {
                    other_driver->set_rigid_body(other_next_vehicle);
                    #pragma GCC diagnostic push
                    #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
                    if (other_ec_old) {
                    #pragma GCC diagnostic pop
                        other_driver->create_externals();
                    }
                }
            }
        }
    }
}
