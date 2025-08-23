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
#include <Mlib/Players/Game_Logic/Spawner.hpp>
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
    Spawner& spawner,
    GameLogicConfig& cfg)
    : current_bystander_rng_{ 0 }
    , current_bvh_rng_{ 0 }
    , spawn_point_rng_{ 0 }
    , current_bvh_{ 0 }
    , vehicle_spawners_{ vehicle_spawners }
    , players_{ players }
    , scene_{ scene }
    , spawner_{ spawner }
    , cfg_{ cfg }
{}

Bystanders::~Bystanders() = default;

struct SpawnPointAndDistance {
    const SpawnPoint* sp;
    ScenePos dist2;
};

/** Spawn player `player` for VIP with orientation `vpi_z` and position `vpi_pos`.
 */
bool Bystanders::spawn_for_vip(
    VehicleSpawner& spawner,
    const std::vector<VipAndPosition>& vips)
{
    // assert_true(player.game_mode() == GameMode::BYSTANDER);
    if (spawner.has_scene_vehicle()) {
        THROW_OR_ABORT("Spawner already has a vehicle");
    }
    std::vector<SpawnPointAndDistance> neighboring_spawn_points;
    neighboring_spawn_points.reserve(1000);
    std::unordered_set<const SpawnPoint*> neighboring_spawn_points_set;
    for (const auto& vip : vips) {
        if (spawner.has_player() && (&spawner.get_player().get() == &vip.player)) {
            THROW_OR_ABORT("Attempt to spawn a VIP as a bystander");
        }
        spawner_.spawn_points_bvh_split_.at(current_bvh_)->visit(
            AxisAlignedBoundingBox<CompressedScenePos, 3>::from_center_and_radius(
                vip.position.casted<CompressedScenePos>(),
                cfg_.r_spawn_far),
            [&](const SpawnPoint* sp)
            {
                if (neighboring_spawn_points_set.contains(sp)) {
                    return true;
                }
                if (spawner.has_player()) {
                    if ((sp->type == SpawnPointType::PARKING) != spawner.get_player()->is_parking()) {
                        return true;
                    }
                    if ((sp->location == WayPointLocation::SIDEWALK) != spawner.get_player()->is_pedestrian()) {
                        return true;
                    }
                }
                ScenePos dist2 = sum(squared(funpack(sp->trafo.t) - vip.position));
                // Abort if too far away.
                if (dist2 > squared(cfg_.r_spawn_far)) {
                    return true;
                }
                // Abort if behind car.
                if (dot0d(funpack(sp->trafo.t) - vip.position, vip.dir_z.casted<ScenePos>()) > 0) {
                    return true;
                }
                neighboring_spawn_points.emplace_back(sp, dist2);
                assert_true(neighboring_spawn_points_set.insert(sp).second);
                return true;
                });
    }
    std::shuffle(neighboring_spawn_points.begin(), neighboring_spawn_points.end(), spawn_point_rng_);
    bool success = false;
    for (size_t i = 0; i < std::min(cfg_.spawn_points_visited_max, neighboring_spawn_points.size()); ++i) {
        const auto& n = neighboring_spawn_points[i];
        // Abort if another car is nearby.
        if ([&](){
            for (const auto& [_, player2] : players_.players()) {
                if (player2->has_scene_vehicle()) {
                    if (sum(squared(funpack(n.sp->trafo.t) - player2->scene_node()->position())) < squared(cfg_.r_neighbors)) {
                        return true;
                    }
                }
            }
            return false;
        }())
        {
            continue;
        }
        auto velocity = -n.sp->trafo.R.column(2) * cfg_.velocity_after_spawn;
        bool spotted = [&](){
            for (const auto& vip : vips_) {
                if (vip->can_see(
                    funpack(n.sp->trafo.t),
                    velocity,
                    cfg_.only_terrain,
                    funpack(cfg_.spawn_point_can_be_seen_y_offset)))
                {
                    return true;
                }
            }
            return false;
        }();
        if (n.dist2 < squared(cfg_.r_spawn_near)) {
            // The spawn point is near the VIP.

            // Abort if visible.
            if (spotted) {
                continue;
            }
            // Abort if not visible after x seconds.
            if (![&](){
                for (const auto& vip : vips_) {
                    if (vip->can_see(
                        funpack(n.sp->trafo.t),
                        velocity,
                        cfg_.only_terrain,
                        funpack(cfg_.spawn_point_can_be_seen_y_offset),
                        cfg_.visible_after_spawn_time))
                    {
                        return true;
                    }
                }
                return false;
            }())
            {
                continue;
            }
        } else {
            // The spawn point is far away from the VIP.

            // Abort if not visible.
            if (!spotted) {
                continue;
            }
        }
        if (spawner_.try_spawn_at_spawn_point(spawner, n.sp->trafo)) {
            if (spotted) {
                spawner.notify_spotted_by_vip();
            }
            success = true;
        }
        break;
    }
    // current_bvh_ = (current_bvh_ + 1) % spawn_.spawn_points_bvhs_split_.size();
    current_bvh_ = current_bvh_rng_() % spawner_.spawn_points_bvh_split_.size();
    return success;
}

bool Bystanders::delete_for_vip(
    VehicleSpawner& spawner,
    const std::vector<VipAndPosition>& vips)
{
    if (!spawner.has_scene_vehicle()) {
        THROW_OR_ABORT("Spawner has no scene vehicle");
    }
    size_t ndelete_votes = 0;
    for (const auto& vehicle : spawner.get_scene_vehicles()) {
        FixedArray<ScenePos, 3> player_pos = vehicle->scene_node()->position();
        for (const auto& vip : vips) {
            ScenePos dist2 = sum(squared(player_pos - vip.position));
            if (dist2 > squared(cfg_.r_delete_near)) {
                // Abort if behind car.
                if (dot0d(player_pos - vip.position, vip.dir_z.casted<ScenePos>()) > 0) {
                    ++ndelete_votes;
                    continue;
                }
            }
            if (dist2 > squared(cfg_.r_delete_far)) {
                if (!vip.player.can_see(
                    vehicle.get(),
                    cfg_.only_terrain))
                {
                    ++ndelete_votes;
                    continue;
                } else {
                    spawner.notify_spotted_by_vip();
                }
            }
            if ((spawner.get_time_since_spawn() > cfg_.visible_after_delete_time) &&
                (spawner.get_time_since_spotted_by_vip() > cfg_.visible_after_spotted_time))
            {
                if (!vip.player.can_see(
                    vehicle.get(),
                    cfg_.only_terrain))
                {
                    ++ndelete_votes;
                    continue;
                } else {
                    spawner.notify_spotted_by_vip();
                }
            }
        }
    }
    if (ndelete_votes == spawner.get_scene_vehicles().size() * vips.size()) {
        // TimeGuard time_guard{"delete", "delete"};
        // std::scoped_lock lock{ delete_node_mutex_ };
        for (const auto& v : spawner.get_scene_vehicles()) {
            scene_.schedule_delete_root_node(v->scene_node_name());
        }
        ++spawner_.ndelete_;
        return true;
    } else {
        return false;
    }
}

void Bystanders::add_vip(const DanglingBaseClassRef<Player>& vip, SourceLocation loc) {
    vips_.emplace_back(vip, loc);
}

void Bystanders::handle_bystanders() {
    std::vector<VipAndPosition> vips;
    vips.reserve(vips_.size());
    for (const auto& vip : vips_) {
        if (!vip->has_scene_vehicle()) {
            continue;
        }
        TransformationMatrix<float, ScenePos, 3> vip_m = vip->scene_node()->absolute_model_matrix();
        const FixedArray<ScenePos, 3>& vip_pos = vip_m.t;
        FixedArray<float, 3> vip_z = z3_from_3x3(vip_m.R);
        vips.emplace_back(vip.get(), vip_z, vip_pos);
    }
    if (vips.empty()) {
        return;
    }
    std::vector<VehicleSpawner*> spawners;
    spawners.reserve(vehicle_spawners_.spawners().size());
    for (auto& [_, spawner] : vehicle_spawners_.spawners()) {
        if (spawner->get_spawn_trigger() != SpawnTrigger::BYSTANDERS) {
            continue;
        }
        if (!spawner->has_scene_vehicle() && !spawner->dependencies_are_met()) {
            continue;
        }
        // if (spawner.has_player()) {
        //     auto player = spawner.get_player();
        //     for (const auto& vip : vips) {
        //         if (&player.get() == &vip.player) {
        //             continue;
        //         }
        //     }
        //     if (player->player_role() != PlayerRole::BYSTANDER) {
        //         // return;
        //         THROW_OR_ABORT("Spawn trigger is \"bystanders\", but player role is not");
        //     }
        // }
        if (std::ranges::any_of(vips.begin(), vips.end(),
            [&spawner](const auto& vip){ return &vip.player.vehicle_spawner() == spawner.get(); }))
        {
            continue;
        }
        spawners.push_back(spawner.get());
    }
    auto it = spawners.begin();
    using players_map_difference_type = decltype(spawners.begin())::difference_type;
    std::advance(it, integral_cast<players_map_difference_type>(current_bystander_rng_() % spawners.size()));
    auto handle_bystander = [&](VehicleSpawner& spawner) {
        if (!spawner.has_scene_vehicle()) {
            spawn_for_vip(spawner, vips);
        } else {
            delete_for_vip(spawner, vips);
        }
    };
    handle_bystander(**it);
}
