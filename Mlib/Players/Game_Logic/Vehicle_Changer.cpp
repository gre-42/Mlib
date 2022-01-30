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
        auto next_rb = p->next_rigid_body();
        if ((next_rb != nullptr) &&
            (!p->has_rigid_body() || (next_rb != &p->rigid_body())))
        {
            if (p->has_rigid_body()) {
                if (next_rb->driver_ == nullptr) {
                    throw std::runtime_error("Next rigid body has no driver");
                }
                Player* other_driver = dynamic_cast<Player*>(next_rb->driver_);
                if (other_driver == nullptr) {
                    throw std::runtime_error("Next vehicle's driver is not a player");
                }
                bool other_ec_old = other_driver->externals_created();
                bool ec_old = p->externals_created();

                PlayerVehicle other_vehicle = other_driver->vehicle();
                PlayerVehicle p_vehicle = p->vehicle();

                other_driver->reset_node();
                p->reset_node();

                other_driver->set_rigid_body(p_vehicle);
                p->set_rigid_body(other_vehicle);

                if (ec_old) {
                    p->create_externals();
                }

                if (other_ec_old) {
                    other_driver->create_externals();
                }
            }
        }
    }
}
