#include "Bystanders.hpp"
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Game_Logic/Game_Logic_Config.hpp>
#include <Mlib/Players/Game_Logic/Spawn.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>

using namespace Mlib;

Bystanders::Bystanders(
    Players& players,
    Scene& scene,
    Spawn& spawn,
    GameLogicConfig& cfg)
: current_bystander_rng_{ 0 },
  current_bvh_rng_{ 0 },
  current_bvh_{ 0 },
  vip_{ nullptr },
  players_{ players },
  scene_{ scene },
  spawn_{ spawn },
  cfg_{ cfg }
{}

Bystanders::~Bystanders()
{}

bool Bystanders::spawn_for_vip(
    Player& player,
    const FixedArray<float, 3>& vip_z,
    const FixedArray<float, 3>& vip_pos)
{
    bool succees = false;
    spawn_.spawn_points_bvhs_[current_bvh_]->visit({vip_pos, cfg_.r_spawn_far}, [&](const SpawnPoint* sp){
        if ((sp->type == SpawnPointType::PARKING) == player.has_waypoints()) {
            return true;
        }
        if ((sp->location == WayPointLocation::SIDEWALK) != player.is_pedestrian()) {
            return true;
        }
        float dist2 = sum(squared(sp->position - vip_pos));
        // Abort if too far away.
        if (dist2 > squared(cfg_.r_spawn_far)) {
            return true;
        }
        // Abort if behind car.
        if (dot0d(sp->position - vip_pos, vip_z) > 0) {
            return true;
        }
        // Abort if another car is nearby.
        {
            bool exists = false;
            for (auto& player2 : players_.players()) {
                if (!player2.second->scene_node_name().empty()) {
                    if (sum(squared(sp->position - scene_.get_node(player2.second->scene_node_name())->position())) < squared(cfg_.r_neighbors)) {
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
                cfg_.visible_after_spawn))
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
        spawn_.spawn_at_spawn_point(player, *sp);
        player.notify_spawn();
        if (spotted) {
            player.set_spotted_by_vip();
        }
        succees = true;
        return false;
    });
    // current_bvh_ = (current_bvh_ + 1) % spawn_points_bvhs_.size();
    current_bvh_ = current_bvh_rng_() % spawn_.spawn_points_bvhs_.size();
    return succees;
}

bool Bystanders::delete_for_vip(
    Player& player,
    const FixedArray<float, 3>& vip_z,
    const FixedArray<float, 3>& vip_pos)
{
    if (!player.has_rigid_body()) {
        return false;
    }
    FixedArray<float, 3> player_pos = scene_.get_node(player.scene_node_name())->position();
    float dist2 = sum(squared(player_pos - vip_pos));
    if (dist2 > squared(cfg_.r_delete_near)) {
        // Abort if behind car.
        if (dot0d(player_pos - vip_pos, vip_z) > 0) {
            goto delete_player;
        }
    }
    if (dist2 > squared(cfg_.r_delete_far)) {
        if (!vip_->can_see(
            player,
            cfg_.only_terrain,
            cfg_.can_see_y_offset))
        {
            goto delete_player;
        } else {
            player.set_spotted_by_vip();
        }
    }
    if (!player.spotted_by_vip() && (player.seconds_since_spawn() > cfg_.visible_after_delete)) {
        if (!vip_->can_see(
            player,
            cfg_.only_terrain,
            cfg_.can_see_y_offset))
        {
            goto delete_player;
        } else {
            player.set_spotted_by_vip();
        }
    }
    return false;
delete_player:
    // TimeGuard time_guard{"delete", "delete"};
    // std::lock_guard lock{ delete_node_mutex_ };
    scene_.clear_nodes_not_allowed_to_be_unregistered();
    scene_.schedule_delete_root_node(player.scene_node_name());
    // ++ndelete_;
    return true;
}

void Bystanders::set_vip(Player* vip) {
    vip_ = vip;
}

void Bystanders::handle_bystanders() {
    if (vip_ == nullptr) {
        return;
    }
    if (!vip_->has_rigid_body()) {
        return;
    }
    if (players_.players().empty()) {
        return;
    }
    TransformationMatrix<float, 3> vip_m = scene_.get_node(vip_->scene_node_name())->absolute_model_matrix();
    const FixedArray<float, 3>& vip_pos = vip_m.t();
    FixedArray<float, 3> vip_z = z3_from_3x3(vip_m.R());
    auto it = players_.players().begin();
    std::advance(it, current_bystander_rng_() % players_.players().size());
    auto handle_bystander = [&](Player& player) {
        if (&player == vip_) {
            return;
        }
        if (player.game_mode() != GameMode::BYSTANDER) {
            return;
        }
        if (player.scene_node_name().empty()) {
            spawn_for_vip(
                player,
                vip_z,
                vip_pos);
        } else {
            delete_for_vip(
                player,
                vip_z,
                vip_pos);
        }
    };
    handle_bystander(*it->second);
}
