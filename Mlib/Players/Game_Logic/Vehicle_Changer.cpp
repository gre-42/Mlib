#include "Vehicle_Changer.hpp"
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Externals_Mode.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

VehicleChanger::VehicleChanger(
    VehicleSpawners& vehicle_spawners,
    DeleteNodeMutex& delete_node_mutex)
    : vehicle_spawners_{ vehicle_spawners }
    , delete_node_mutex_{ delete_node_mutex }
{}

void VehicleChanger::change_vehicles() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    for (const auto& [_, s] : vehicle_spawners_.spawners()) {
        if (!s->has_player()) {
            continue;
        }
        auto p = s->get_player();
        auto next_vehicle = p->next_scene_vehicle();
        if (next_vehicle == nullptr) {
            continue;
        }
        if (!p->has_scene_vehicle()) {
            continue;
        }
        if (next_vehicle->get_primary_scene_vehicle()->scene_node().ptr() == p->scene_node().ptr()) {
            THROW_OR_ABORT("Next scene node equals current node");
        }
        auto& next_rb = get_rigid_body_vehicle(next_vehicle->get_primary_scene_vehicle()->scene_node());
        auto* other_player = next_rb.drivers_.try_get(p->next_seat()).get();
        if (other_player == nullptr) {
            enter_vehicle(*s, *next_vehicle);
        } else {
            auto* other_driver = dynamic_cast<Player*>(other_player);
            if (other_driver == nullptr) {
                THROW_OR_ABORT("Next vehicle's driver is not a player");
            }
            swap_vehicles(p.get(), *other_driver);
        }
    }
}

void VehicleChanger::swap_vehicles(Player& a, Player& b) {
    ExternalsMode b_ec_old = b.externals_mode();
    ExternalsMode a_ec_old = a.externals_mode();
    InternalsMode b_seat_old = b.internals_mode();
    InternalsMode a_seat_old = a.internals_mode();

    VehicleSpawner& b_spawner = b.vehicle_spawner();
    VehicleSpawner& a_spawner = a.vehicle_spawner();

    b.reset_node();
    a.reset_node();

    b.set_vehicle_spawner(a_spawner, b.next_seat());
    a.set_vehicle_spawner(b_spawner, a.next_seat());

    if (a_ec_old != ExternalsMode::NONE) {
        a.create_vehicle_externals(a_ec_old);
    }
    if (!a_seat_old.seat.empty()) {
        a.create_vehicle_internals(a_seat_old);
    }

    if (b_ec_old != ExternalsMode::NONE) {
        b.create_vehicle_externals(b_ec_old);
    }
    if (!b_seat_old.seat.empty()) {
        b.create_vehicle_internals(b_seat_old);
    }
}

void VehicleChanger::enter_vehicle(VehicleSpawner& a, VehicleSpawner& b) {
    if (!a.has_player()) {
        THROW_OR_ABORT("Vehicle spawner has no player");
    }
    auto b_rb = b.get_primary_scene_vehicle()->rb();
    if (b_rb->is_avatar() && (&a.get_primary_scene_vehicle().get() != &b.get_primary_scene_vehicle().get())) {
        THROW_OR_ABORT("Can only enter the initial avatar");
    }
    auto ap = a.get_player();
    if (&ap->vehicle().get() == &b.get_primary_scene_vehicle().get()) {
        THROW_OR_ABORT("Entering the same vehicle");
    }
    auto a_rb_old = ap->rigid_body();
    if (a_rb_old->is_avatar()) {
        a_rb_old->deactivate_avatar();
    } else if (a_rb_old->passengers_.erase(a.get_primary_scene_vehicle()->rb().ptr()) != 1) {
        THROW_OR_ABORT("Could not find passenger to be deleted");
    }
    if (b_rb->is_activated_avatar()) {
        THROW_OR_ABORT("Destination avatar is not deactivated");
    }
    if (b_rb->is_deactivated_avatar()) {
        if (std::isnan(a_rb_old->door_distance_)) {
            THROW_OR_ABORT("Door distance not set");
        }
        auto a_trafo = a_rb_old->rbp_.abs_transformation();
        FixedArray<float, 3> a_dir = (std::abs(a_trafo.R(0, 1)) > 0.9f)
            ? a_trafo.R.column(2)
            : a_trafo.R.column(0);
        a_dir(1) = 0.f;
        a_dir /= std::sqrt(sum(squared(a_dir)));
        // Subtract PI/2 because we want to set the angle in z-direction,
        // while the angle computed by atan2 is measured along the x-direction.
        float angle = std::atan2(-a_dir(2), a_dir(0)) - float(M_PI / 2.);
        b_rb->rbp_.set_pose(
            tait_bryan_angles_2_matrix(FixedArray<float, 3>{0.f, angle, 0.f}),
            a_trafo.t + (a_rb_old->door_distance_ * a_dir).casted<ScenePos>());
        b_rb->rbp_.v_com_ = 0.f;
        b_rb->rbp_.w_ = 0.f;
        b.get_primary_scene_vehicle()->scene_node()->invalidate_transformation_history();
        b_rb->activate_avatar();
    }
    ExternalsMode a_ec_old = ap->externals_mode();
    auto a_seat_old = ap->internals_mode();
    ap->reset_node();
    ap->set_vehicle_spawner(b, ap->next_seat());
    ap->create_vehicle_externals(a_ec_old);
    if (!a_seat_old.seat.empty()) {
        ap->create_vehicle_internals(a_seat_old);
    }
    if (!ap->rigid_body()->is_avatar()) {
        if (!ap->rigid_body()->passengers_.try_emplace(
            a.get_primary_scene_vehicle()->rb().set_loc(CURRENT_SOURCE_LOCATION).ptr(),
            CURRENT_SOURCE_LOCATION).second)
        {
            THROW_OR_ABORT("Passenger already exists");
        }
    }
}
