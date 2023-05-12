#include "Bystanders.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Game_Logic/Game_Logic_Config.hpp>
#include <Mlib/Players/Game_Logic/Spawn.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>

using namespace Mlib;

Bystanders::Bystanders(
    VehicleSpawners& vehicle_spawners,
    Players& players,
    Scene& scene,
    Spawn& spawn,
    GameLogicConfig& cfg)
: current_bystander_rng_{ 0 },
  current_bvh_rng_{ 0 },
  current_bvh_{ 0 },
  vip_{ nullptr },
  vehicle_spawners_{ vehicle_spawners },
  players_{ players },
  scene_{ scene },
  spawn_{ spawn },
  cfg_{ cfg }
{}

Bystanders::~Bystanders()
{}

/** Spawn player `player` for VIP with orientation `vpi_z` and position `vpi_pos`.
 */
bool Bystanders::spawn_for_vip(
    VehicleSpawner& spawner,
    const FixedArray<float, 3>& vip_z,
    const FixedArray<double, 3>& vip_pos)
{
    // assert_true(player.game_mode() == GameMode::BYSTANDER);
    if (spawner.has_scene_vehicle()) {
        THROW_OR_ABORT("Spawner already has a vehicle");
    }
    bool success = false;
    spawn_.spawn_points_bvh_split_.at(current_bvh_)->visit({vip_pos, cfg_.r_spawn_far}, [&](const SpawnPoint* sp){
        if ((sp->type == SpawnPointType::PARKING) == (spawner.has_player() && spawner.get_player().pathfinding_waypoints().has_waypoints())) {
            return true;
        }
        if ((sp->location == WayPointLocation::SIDEWALK) != (spawner.has_player() && spawner.get_player().is_pedestrian())) {
            return true;
        }
        double dist2 = sum(squared(sp->position - vip_pos));
        // Abort if too far away.
        if (dist2 > squared(cfg_.r_spawn_far)) {
            return true;
        }
        // Abort if behind car.
        if (dot0d(sp->position - vip_pos, vip_z.casted<double>()) > 0) {
            return true;
        }
        // Abort if another car is nearby.
        {
            bool exists = false;
            for (auto const& [_, player2] : players_.players()) {
                if (!player2->scene_node_name().empty()) {
                    if (sum(squared(sp->position - scene_.get_node(player2->scene_node_name()).position())) < squared(cfg_.r_neighbors)) {
                        exists = true;
                        break;
                    }
                }
            }
            if (exists) {
                return true;
            }
        }
        bool spotted = vip_->can_see(
            sp->position,
            cfg_.only_terrain,
            cfg_.can_see_y_offset);
        if (dist2 < squared(cfg_.r_spawn_near)) {
            // The spawn point is near the VIP.

            // Abort if visible.
            if (spotted) {
                return true;
            }
            // Abort if not visible after x seconds.
            if (!vip_->can_see(
                sp->position,
                cfg_.only_terrain,
                cfg_.can_see_y_offset,
                cfg_.visible_after_spawn_time))
            {
                return true;
            }
        } else {
            // The spawn point is far away from the VIP.

            // Abort if not visible.
            if (!spotted) {
                return true;
            }
        }
        spawn_.spawn_at_spawn_point(spawner, *sp);
        if (spotted) {
            spawner.set_spotted_by_vip();
        }
        success = true;
        return false;
    });
    // current_bvh_ = (current_bvh_ + 1) % spawn_.spawn_points_bvhs_split_.size();
    current_bvh_ = current_bvh_rng_() % spawn_.spawn_points_bvh_split_.size();
    return success;
}

bool Bystanders::delete_for_vip(
    VehicleSpawner& spawner,
    const FixedArray<float, 3>& vip_z,
    const FixedArray<double, 3>& vip_pos)
{
    if (!spawner.has_scene_vehicle()) {
        THROW_OR_ABORT("Spawner has not scene vehicle");
    }
    auto& vehicle = spawner.get_scene_vehicle();
    FixedArray<double, 3> player_pos = vehicle.scene_node().position();
    double dist2 = sum(squared(player_pos - vip_pos));
    if (dist2 > squared(cfg_.r_delete_near)) {
        // Abort if behind car.
        if (dot0d(player_pos - vip_pos, vip_z.casted<double>()) > 0) {
            goto delete_player;
        }
    }
    if (dist2 > squared(cfg_.r_delete_far)) {
        if (!vip_->can_see(
            vehicle,
            cfg_.only_terrain,
            cfg_.can_see_y_offset))
        {
            goto delete_player;
        } else {
            spawner.set_spotted_by_vip();
        }
    }
    if (!spawner.spotted_by_vip() && (spawner.seconds_since_spawn() > cfg_.visible_after_delete_seconds)) {
        if (!vip_->can_see(
            vehicle,
            cfg_.only_terrain,
            cfg_.can_see_y_offset))
        {
            goto delete_player;
        } else {
            spawner.set_spotted_by_vip();
        }
    }
    return false;
delete_player:
    // TimeGuard time_guard{"delete", "delete"};
    // std::scoped_lock lock{ delete_node_mutex_ };
    scene_.clear_nodes_not_allowed_to_be_unregistered();
    scene_.schedule_delete_root_node(vehicle.scene_node_name());
    ++spawn_.ndelete_;
    return true;
}

void Bystanders::set_vip(Player* vip) {
    vip_ = vip;
}

void Bystanders::handle_bystanders() {
    if (vip_ == nullptr) {
        return;
    }
    if (!vip_->has_scene_vehicle()) {
        return;
    }
    if (players_.players().empty()) {
        return;
    }
    TransformationMatrix<float, double, 3> vip_m = scene_.get_node(vip_->scene_node_name()).absolute_model_matrix();
    const FixedArray<double, 3>& vip_pos = vip_m.t();
    FixedArray<float, 3> vip_z = z3_from_3x3(vip_m.R());
    auto it = vehicle_spawners_.spawners().begin();
    using players_map_difference_type = decltype(players_.players().begin())::difference_type;
    std::advance(it, integral_cast<players_map_difference_type>(current_bystander_rng_() % players_.players().size()));
    auto handle_bystander = [&](VehicleSpawner& spawner) {
        if (spawner.has_player()) {
            auto& player = spawner.get_player();
            if (spawner.has_player() && (&player == vip_)) {
                return;
            }
            if (player.game_mode() != GameMode::BYSTANDER) {
                return;
            }
        }
        if (!spawner.has_scene_vehicle()) {
            spawn_for_vip(
                spawner,
                vip_z,
                vip_pos);
        } else {
            delete_for_vip(
                spawner,
                vip_z,
                vip_pos);
        }
    };
    handle_bystander(*it->second);
}
