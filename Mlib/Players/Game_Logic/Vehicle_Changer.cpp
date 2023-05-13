#include "Vehicle_Changer.hpp"
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Scene_Vehicle/Externals_Mode.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

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
        auto next_vehicle = p->next_scene_vehicle();
        if (next_vehicle == nullptr) {
            continue;
        }
        if (!p->has_scene_vehicle()) {
            continue;
        }
        if (&next_vehicle->scene_node() == &p->scene_node()) {
            THROW_OR_ABORT("Next scene node equals current node");
        }
        auto* next_rb = dynamic_cast<RigidBodyVehicle*>(&next_vehicle->scene_node().get_absolute_movable());
        if (next_rb == nullptr) {
            THROW_OR_ABORT("Next movable is no rigid body");
        }
        if (next_rb->driver_ == nullptr) {
            enter_vehicle(*p, *next_vehicle);
        } else {
            Player* other_driver = dynamic_cast<Player*>(next_rb->driver_);
            if (other_driver == nullptr) {
                THROW_OR_ABORT("Next vehicle's driver is not a player");
            }
            swap_vehicles(*p, *other_driver);
        }
    }
}

void VehicleChanger::swap_vehicles(Player& a, Player& b) {
    ExternalsMode b_ec_old = b.externals_mode();
    ExternalsMode a_ec_old = a.externals_mode();

    SceneVehicle& b_vehicle = b.vehicle();
    SceneVehicle& a_vehicle = a.vehicle();

    b.reset_node();
    a.reset_node();

    b.set_scene_vehicle(a_vehicle);
    a.set_scene_vehicle(b_vehicle);

    if (a_ec_old != ExternalsMode::NONE) {
        a.create_externals(a_ec_old);
    }

    if (b_ec_old != ExternalsMode::NONE) {
        b.create_externals(b_ec_old);
    }
}

void VehicleChanger::enter_vehicle(Player& a, SceneVehicle& b) {
    ExternalsMode a_ec_old = a.externals_mode();
    a.reset_node();
    a.set_scene_vehicle(b);
    a.create_externals(a_ec_old);
}
