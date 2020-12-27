#include "Game_Logic.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Physics/Advance_Times/Player.hpp>
#include <Mlib/Physics/Containers/Players.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <stdexcept>

using namespace Mlib;

GameLogic::GameLogic(
    Scene& scene,
    Players& players,
    const std::list<Focus>& focus,
    std::recursive_mutex& mutex)
: scene_{scene},
  players_{players},
  vip_{nullptr},
  focus_{focus},
  mutex_{mutex},
  spawn_point_id_{0}
{}

void GameLogic::set_spawn_points(const SceneNode& node, const std::list<SpawnPoint>& spawn_points) {
    spawn_points_ = std::vector(spawn_points.begin(), spawn_points.end());
    FixedArray<float, 4, 4> m = node.absolute_model_matrix();
    FixedArray<float, 3, 3> r = R3_from_4x4(m) / node.scale();
    for (SpawnPoint& p : spawn_points_) {
        p.position = dehomogenized_3(dot1d(m, homogenized_4(p.position)));
        p.rotation = matrix_2_tait_bryan_angles(dot2d(dot2d(r, tait_bryan_angles_2_matrix(p.rotation)), r.T()));
    }
}

void GameLogic::set_preferred_car_spawner(Player& player, const std::function<void(const SpawnPoint&)>& preferred_car_spawner) {
    preferred_car_spawners_.insert_or_assign(&player, preferred_car_spawner);
}

void GameLogic::advance_time(float dt) {
    handle_team_deathmatch();
    handle_bystanders();
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
        std::lock_guard lock_guard{mutex_};
        for (auto& p : players_.players()) {
            if (p.second->team() == *winner_teams.begin()) {
                ++p.second->stats().nwins;
            }
        }
        for (auto& p : players_.players()) {
            const std::string& node_name = p.second->scene_node_name();
            if (!node_name.empty()) {
                scene_.delete_root_node(node_name);
            }
        }
        auto sit = spawn_points_.begin();
        auto pit = players_.players().begin();
        for (; sit != spawn_points_.end() && pit != players_.players().end(); ++sit, ++pit) {
            auto it = preferred_car_spawners_.find(pit->second);
            if (it == preferred_car_spawners_.end()) {
                throw std::runtime_error("Player " + pit->second->name() + " has no preferred car spawner");
            }
            it->second(*sit);
        }
    }
}

void GameLogic::handle_bystanders() {
    if (vip_ == nullptr) {
        return;
    }
    if (vip_->scene_node_name().empty()) {
        return;
    }
    FixedArray<float, 4, 4> vip_m = scene_.get_node(vip_->scene_node_name())->absolute_model_matrix();
    FixedArray<float, 3> vip_pos = t3_from_4x4(vip_m);
    FixedArray<float, 3> vip_z = z3_from_4x4(vip_m);
    float r_spawn = 200;
    float r_delete = 300;
    float r_neighbors = 20;
    float visible_after_spawn = 2;
    float visible_after_delete = 3;
    size_t max_nsee = 5;
    size_t nsee = 0;
    for (auto& player : players_.players()) {
        if (player.second == vip_) {
            continue;
        }
        auto it = preferred_car_spawners_.find(player.second);
        if (it == preferred_car_spawners_.end()) {
            throw std::runtime_error("Player " + player.second->name() + " has no preferred car spawner");
        }
        if (player.second->game_mode() == GameMode::BYSTANDER) {
            if (player.second->scene_node_name().empty()) {
                if (nsee > max_nsee) {
                    continue;
                }
                bool spawned = false;
                for (size_t i = 0; i < spawn_points_.size(); ++i) {
                    const SpawnPoint& sp = spawn_points_[(spawn_point_id_++) % spawn_points_.size()];
                    // Abort if too far away.
                    if (sum(squared(sp.position - vip_pos)) > squared(r_spawn)) {
                        continue;
                    }
                    // Abort if behind car.
                    if (dot0d(sp.position - vip_pos, vip_z) > 0) {
                        continue;
                    }
                    // Abort if another car is nearby.
                    {
                        bool exists = false;
                        for (auto& player2 : players_.players()) {
                            if (!player2.second->scene_node_name().empty()) {
                                if (sum(squared(sp.position - scene_.get_node(player2.second->scene_node_name())->position())) < squared(r_neighbors)) {
                                    exists = true;
                                    break;
                                }
                            }
                        }
                        if (exists) {
                            continue;
                        }
                    }
                    ++nsee;
                    if (nsee > max_nsee) {
                        break;
                    }
                    // Abort if visible.
                    if (vip_->can_see(sp.position, 2)) {
                        continue;
                    }
                    // Abort if not visible after x seconds.
                    if (!vip_->can_see(sp.position, 2, visible_after_spawn)) {
                        continue;
                    }
                    it->second(sp);
                    if (player.second->scene_node_name().empty()) {
                        throw std::runtime_error("Scene node name empty for player " + player.first);
                    }
                    player.second->notify_spawn();
                    spawned = true;
                    break;
                }
                if (!spawned) {
                    // std::cerr << "Could not spawn bystander " << player.second->name() << std::endl;
                    break;
                }
            } else {
                if (sum(squared(scene_.get_node(player.second->scene_node_name())->position() - vip_pos)) > squared(r_delete)) {
                    goto delete_player;
                }
                if (!player.second->spotted() && (player.second->seconds_since_spawn() > visible_after_delete)) {
                    if (!vip_->can_see(*player.second)) {
                        goto delete_player;
                    } else {
                        player.second->set_spotted();
                    }
                }
                goto nodelete_player;
                delete_player:
                {
                    std::lock_guard lock_guard{mutex_};
                    scene_.delete_root_node(player.second->scene_node_name());
                }
                nodelete_player:
                    ;
            }
        }
    }
}

void GameLogic::set_vip(Player* vip) {
    vip_ = vip;
}
