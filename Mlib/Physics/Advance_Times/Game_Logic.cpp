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

GameLogic::GameLogic(Scene& scene, Players& players, const std::list<Focus>& focus)
: scene_{scene},
  players_{players},
  focus_{focus}
{}

void GameLogic::set_spawn_points(const SceneNode& node, const std::list<SpawnPoint>& spawn_points) {
    spawn_points_ = spawn_points;
    FixedArray<float, 4, 4> m = node.absolute_model_matrix();
    FixedArray<float, 3, 3> r = R3_from_4x4(m) / node.scale();
    for(SpawnPoint& p : spawn_points_) {
        p.position = dehomogenized_3(dot1d(m, homogenized_4(p.position)));
        p.rotation = matrix_2_tait_bryan_angles(dot2d(r, tait_bryan_angles_2_matrix(p.rotation)));
    }
}

void GameLogic::set_preferred_car_spawner(Player& player, const std::function<void(const SpawnPoint&)>& preferred_car_spawner) {
    preferred_car_spawners_.insert_or_assign(&player, preferred_car_spawner);
}

void GameLogic::advance_time(float dt) {
    size_t nwinners = 0;
    Player* winner = nullptr;
    for(auto& p : players_.players()) {
        const std::string& node_name = p.second->scene_node_name();
        if (!node_name.empty()) {
            ++nwinners;
            winner = p.second;
        }
    }
    if ((nwinners <= 1) &&
        (players_.players().size() > 1) &&
        (spawn_points_.size() > 1))
    {
        if (winner != nullptr) {
            ++winner->stats().nwins;
        }
        for(auto& p : players_.players()) {
            const std::string& node_name = p.second->scene_node_name();
            if (!node_name.empty()) {
                scene_.delete_root_node(node_name);
            }
        }
        auto sit = spawn_points_.begin();
        auto pit = players_.players().begin();
        for(; sit != spawn_points_.end() && pit != players_.players().end(); ++sit, ++pit) {
            auto it = preferred_car_spawners_.find(pit->second);
            if (it == preferred_car_spawners_.end()) {
                throw std::runtime_error("Player " + pit->second->name() + " has no preferred car spawner");
            }
            it->second(*sit);
        }
    }
}
