#include "Game_Logic.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Physics/Advance_Times/Player.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Containers/Players.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <chrono>
#include <stdexcept>

using namespace Mlib;

struct GameLogicConfig {
    float r_spawn_near = 200;
    float r_spawn_far = 400;
    float r_delete_near = 100;
    float r_delete_far = 300;
    float r_neighbors = 20;
    float visible_after_spawn = 2;
    float visible_after_delete = 3;
    float spawn_y_offset = 0.7f;
    bool only_terrain = true;
    float can_see_y_offset = 2;
    size_t spawn_points_nsubdivisions = 5 * 60;
};

GameLogicConfig cfg;

GameLogic::GameLogic(
    Scene& scene,
    AdvanceTimes& advance_times,
    Players& players,
    std::recursive_mutex& mutex)
: scene_{scene},
  advance_times_{advance_times},
  players_{players},
  vip_{nullptr},
  mutex_{mutex},
  current_bystander_rng_{0},
  current_bvh_rng_{0},
  current_bvh_{0}
{
    advance_times_.add_advance_time(*this);
}

GameLogic::~GameLogic() {
    advance_times_.schedule_delete_advance_time(this);
}

void GameLogic::set_spawn_points(const SceneNode& node, const std::list<SpawnPoint>& spawn_points) {
    spawn_points_.clear();
    spawn_points_.reserve(spawn_points.size());
    TransformationMatrix tm{node.absolute_model_matrix()};
    FixedArray<float, 3, 3> R = tm.R() / node.scale();
    size_t nsubs = cfg.spawn_points_nsubdivisions;
    spawn_points_bvhs_.resize(nsubs);
    for (size_t i = 0; i < nsubs; ++i) {
        spawn_points_bvhs_[i].reset(new Bvh<float, const SpawnPoint*, 3>(FixedArray<float, 3>{100.f, 100.f, 100.f}, 10));
    }
    {
        size_t i = 0;
        for (const auto& sp : spawn_points) {
            SpawnPoint sp2 = sp;
            sp2.position = tm.transform(sp.position);
            sp2.rotation = matrix_2_tait_bryan_angles(dot2d(dot2d(R, tait_bryan_angles_2_matrix(sp.rotation)), R.T()));
            spawn_points_.push_back(sp2);
            spawn_points_bvhs_[i]->insert(sp2.position, &spawn_points_.back());
            i = (i + 1) % nsubs;
        }
    }
}

void GameLogic::set_preferred_car_spawner(Player& player, const std::function<void(const SpawnPoint&)>& preferred_car_spawner) {
    preferred_car_spawners_.insert_or_assign(&player, preferred_car_spawner);
}

void GameLogic::advance_time(float dt) {
    // TimeGuard tg{"GameLogic::advance_time"};
    // nspawns_ = 0;
    // ndelete_ = 0;
    handle_team_deathmatch();
    handle_bystanders();
    // size_t nactive = 0;
    // for (const auto& p : players_.players()) {
    //     nactive += !p.second->scene_node_name().empty();
    // }
    // std::cerr << "nactive " << nactive << std::endl;
    // std::cerr << "nspawns " << nspawns_ << " , ndelete " << ndelete_ << std::endl;
}

void GameLogic::handle_team_deathmatch() {
    std::set<std::string> all_teams;
    std::set<std::string> winner_teams;
    for (auto& p : players_.players()) {
        const std::string& node_name = p.second->scene_node_name();
        all_teams.insert(p.second->team());
        if (!node_name.empty()) {
            winner_teams.insert(p.second->team());
        }
    }
    if ((winner_teams.size() <= 1) &&
        (all_teams.size() > 1) &&
        (spawn_points_.size() > 1))
    {
        {
            // std::lock_guard lock_guard{mutex_};
            for (auto& p : players_.players()) {
                if (p.second->team() == *winner_teams.begin()) {
                    ++p.second->stats().nwins;
                }
            }
            for (auto& p : players_.players()) {
                const std::string& node_name = p.second->scene_node_name();
                if (!node_name.empty()) {
                    scene_.delete_root_node(node_name);
                    // ++ndelete_;
                }
            }
        }
        auto sit = spawn_points_.begin();
        auto pit = players_.players().begin();
        for (; sit != spawn_points_.end() && pit != players_.players().end(); ++sit, ++pit) {
            spawn_at_spawn_point(*pit->second, *sit);
        }
    }
}

void GameLogic::handle_bystanders() {
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

bool GameLogic::spawn_for_vip(
    Player& player,
    const FixedArray<float, 3>& vip_z,
    const FixedArray<float, 3>& vip_pos)
{
    bool succees = false;
    spawn_points_bvhs_[current_bvh_]->visit({vip_pos, cfg.r_spawn_far}, [&](const SpawnPoint* sp){
        if ((sp->type == SpawnPointType::PARKING) == player.has_waypoints()) {
            return true;
        }
        if ((sp->location == WayPointLocation::SIDEWALK) != player.is_pedestrian()) {
            return true;
        }
        float dist2 = sum(squared(sp->position - vip_pos));
        // Abort if too far away.
        if (dist2 > squared(cfg.r_spawn_far)) {
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
                    if (sum(squared(sp->position - scene_.get_node(player2.second->scene_node_name())->position())) < squared(cfg.r_neighbors)) {
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
            cfg.only_terrain,
            cfg.can_see_y_offset);
        if (dist2 < squared(cfg.r_spawn_near)) {
            // The spawn point is near the VIP.

            // Abort if visible.
            if (spotted) {
                return true;
            }
            // Abort if not visible after x seconds.
            if (!vip_->can_see(
                sp->position,
                cfg.only_terrain,
                cfg.can_see_y_offset,
                cfg.visible_after_spawn))
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
        spawn_at_spawn_point(player, *sp);
        player.notify_spawn();
        if (spotted) {
            player.set_spotted();
        }
        succees = true;
        return false;
    });
    // current_bvh_ = (current_bvh_ + 1) % spawn_points_bvhs_.size();
    current_bvh_ = current_bvh_rng_() % spawn_points_bvhs_.size();
    return succees;
}

bool GameLogic::delete_for_vip(
    Player& player,
    const FixedArray<float, 3>& vip_z,
    const FixedArray<float, 3>& vip_pos)
{
    if (!player.has_rigid_body()) {
        return false;
    }
    FixedArray<float, 3> player_pos = scene_.get_node(player.scene_node_name())->position();
    float dist2 = sum(squared(player_pos - vip_pos));
    if (dist2 > squared(cfg.r_delete_near)) {
        // Abort if behind car.
        if (dot0d(player_pos - vip_pos, vip_z) > 0) {
            goto delete_player;
        }
    }
    if (dist2 > squared(cfg.r_delete_far)) {
        if (!vip_->can_see(
            player,
            cfg.only_terrain,
            cfg.can_see_y_offset))
        {
            goto delete_player;
        } else {
            player.set_spotted();
        }
    }
    if (!player.spotted() && (player.seconds_since_spawn() > cfg.visible_after_delete)) {
        if (!vip_->can_see(
            player,
            cfg.only_terrain,
            cfg.can_see_y_offset))
        {
            goto delete_player;
        } else {
            player.set_spotted();
        }
    }
    return false;
    delete_player:
    // TimeGuard time_guard{"delete", "delete"};
    // std::lock_guard lock_guard{mutex_};
    scene_.schedule_delete_root_node(player.scene_node_name());
    // ++ndelete_;
    return true;
}

void GameLogic::set_vip(Player* vip) {
    vip_ = vip;
}

void GameLogic::spawn_at_spawn_point(
    Player& player,
    const SpawnPoint& sp)
{
    if (!player.scene_node_name().empty()) {
        throw std::runtime_error("Before spawning, scene node name not empty for player " + player.name());
    }
    auto spawn_macro = preferred_car_spawners_.find(&player);
    if (spawn_macro == preferred_car_spawners_.end()) {
        throw std::runtime_error("Player " + player.name() + " has no preferred car spawner");
    }
    SpawnPoint sp2 = sp;
    sp2.position(1) += cfg.spawn_y_offset;
    // TimeGuard time_guard{"spawn", "spawn"};
    // std::lock_guard lock_guard{mutex_};
    // TimeGuard time_guard2{"spawn2", "spawn2"};
    // ++nspawns_;
    // auto start = std::chrono::steady_clock::now();
    spawn_macro->second(sp2);
    // std::cerr << "Spawn time " << 1000 * std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count() << std::endl;
    if (player.scene_node_name().empty()) {
        throw std::runtime_error("After spawning, scene node name empty for player " + player.name());
    }
    // while (true) {
    //     std::lock_guard lock_guard{ mutex_ };
    //     spawn_macro->second(sp2);
    //     if (player.scene_node_name().empty()) {
    //         throw std::runtime_error("After spawning, scene node name empty for player " + player.name());
    //     }
    //     scene_.delete_root_node(player.scene_node_name());
    // }
}
