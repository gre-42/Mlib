#include "Bystanders.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Object.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
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
    const FixedArray<ScenePos, 3>& vip_pos)
{
    // assert_true(player.game_mode() == GameMode::BYSTANDER);
    if (spawner.has_scene_vehicle()) {
        THROW_OR_ABORT("Spawner already has a vehicle");
    }
    bool success = false;
    spawn_.spawn_points_bvh_split_.at(current_bvh_)->visit(
        AxisAlignedBoundingBox<CompressedScenePos, 3>::from_center_and_radius(
            vip_pos.casted<CompressedScenePos>(),
            cfg_.r_spawn_far),
        [&](const SpawnPoint* sp)
    {
        if ((sp->type == SpawnPointType::PARKING) == (spawner.has_player() && spawner.get_player()->has_way_points())) {
            return true;
        }
        if ((sp->location == WayPointLocation::SIDEWALK) != (spawner.has_player() && spawner.get_player()->is_pedestrian())) {
            return true;
        }
        ScenePos dist2 = sum(squared(funpack(sp->position) - vip_pos));
        // Abort if too far away.
        if (dist2 > squared(cfg_.r_spawn_far)) {
            return true;
        }
        // Abort if behind car.
        if (dot0d(funpack(sp->position) - vip_pos, vip_z.casted<ScenePos>()) > 0) {
            return true;
        }
        // Abort if another car is nearby.
        {
            bool exists = false;
            for (const auto& [_, player2] : players_.players()) {
                if (player2->has_scene_vehicle()) {
                    if (sum(squared(funpack(sp->position) - player2->scene_node()->position())) < squared(cfg_.r_neighbors)) {
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
            funpack(sp->position),
            cfg_.only_terrain,
            funpack(cfg_.can_see_y_offset));
        if (dist2 < squared(cfg_.r_spawn_near)) {
            // The spawn point is near the VIP.

            // Abort if visible.
            if (spotted) {
                return true;
            }
            // Abort if not visible after x seconds.
            if (!vip_->can_see(
                funpack(sp->position),
                cfg_.only_terrain,
                funpack(cfg_.can_see_y_offset),
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
    const FixedArray<ScenePos, 3>& vip_pos)
{
    if (!spawner.has_scene_vehicle()) {
        THROW_OR_ABORT("Spawner has no scene vehicle");
    }
    size_t ndelete_votes = 0;
    for (const auto& vehicle : spawner.get_scene_vehicles()) {
        FixedArray<ScenePos, 3> player_pos = vehicle->scene_node()->position();
        ScenePos dist2 = sum(squared(player_pos - vip_pos));
        if (dist2 > squared(cfg_.r_delete_near)) {
            // Abort if behind car.
            if (dot0d(player_pos - vip_pos, vip_z.casted<ScenePos>()) > 0) {
                ++ndelete_votes;
                continue;
            }
        }
        if (dist2 > squared(cfg_.r_delete_far)) {
            if (!vip_->can_see(
                *vehicle,
                cfg_.only_terrain,
                funpack(cfg_.can_see_y_offset)))
            {
                ++ndelete_votes;
                continue;
            } else {
                spawner.set_spotted_by_vip();
            }
        }
        if (!spawner.get_spotted_by_vip() && (spawner.get_time_since_spawn() > cfg_.visible_after_delete_time)) {
            if (!vip_->can_see(
                *vehicle,
                cfg_.only_terrain,
                funpack(cfg_.can_see_y_offset)))
            {
                ++ndelete_votes;
                continue;
            } else {
                spawner.set_spotted_by_vip();
            }
        }
    }
    if (ndelete_votes == spawner.get_scene_vehicles().size()) {
        // TimeGuard time_guard{"delete", "delete"};
        // std::scoped_lock lock{ delete_node_mutex_ };
        for (const auto& v : spawner.get_scene_vehicles()) {
            scene_.schedule_delete_root_node(v->scene_node_name());
        }
        ++spawn_.ndelete_;
        return true;
    } else {
        return false;
    }
}

void Bystanders::set_vip(const DanglingBaseClassPtr<Player>& vip) {
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
    TransformationMatrix<float, ScenePos, 3> vip_m = vip_->scene_node()->absolute_model_matrix();
    const FixedArray<ScenePos, 3>& vip_pos = vip_m.t;
    FixedArray<float, 3> vip_z = z3_from_3x3(vip_m.R);
    auto it = vehicle_spawners_.spawners().begin();
    using players_map_difference_type = decltype(vehicle_spawners_.spawners().begin())::difference_type;
    std::advance(it, integral_cast<players_map_difference_type>(current_bystander_rng_() % vehicle_spawners_.spawners().size()));
    auto handle_bystander = [&](VehicleSpawner& spawner) {
        if (spawner.has_player()) {
            auto player = spawner.get_player();
            if (spawner.has_player() && (&player.get() == vip_.get())) {
                return;
            }
            if (player->game_mode() != GameMode::BYSTANDER) {
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
