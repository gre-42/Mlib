#include "Spawn.hpp"
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Game_Logic/Game_Logic_Config.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>

using namespace Mlib;

Spawn::Spawn(
    Players& players,
    GameLogicConfig& cfg,
    DeleteNodeMutex& delete_node_mutex,
    Scene& scene)
: players_{ players },
  cfg_{ cfg },
  delete_node_mutex_{ delete_node_mutex },
  scene_{ scene }
{}

Spawn::~Spawn()
{}

void Spawn::set_spawn_points(const SceneNode& node, const std::list<SpawnPoint>& spawn_points) {
    spawn_points_.clear();
    spawn_points_.reserve(spawn_points.size());
    TransformationMatrix tm{node.absolute_model_matrix()};
    FixedArray<float, 3, 3> R = tm.R() / node.scale();
    size_t nsubs = cfg_.spawn_points_nsubdivisions;
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

void Spawn::set_preferred_car_spawner(Player& player, const std::function<void(const SpawnPoint&)>& preferred_car_spawner) {
    preferred_car_spawners_.insert_or_assign(&player, preferred_car_spawner);
}

void Spawn::spawn_at_spawn_point(
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
    sp2.position(1) += cfg_.spawn_y_offset;
    // TimeGuard time_guard{"spawn", "spawn"};
    // std::lock_guard lock{ delete_node_mutex_ };
    // TimeGuard time_guard2{"spawn2", "spawn2"};
    ++nspawns_;
    // auto start = std::chrono::steady_clock::now();
    spawn_macro->second(sp2);
    // std::cerr << "Spawn time " << 1000 * std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count() << std::endl;
    if (player.scene_node_name().empty()) {
        throw std::runtime_error("After spawning, scene node name empty for player " + player.name());
    }
    player.notify_spawn();
    // while (true) {
    //     std::lock_guard lock{ delete_node_mutex_ };
    //     spawn_macro->second(sp2);
    //     if (player.scene_node_name().empty()) {
    //         throw std::runtime_error("After spawning, scene node name empty for player " + player.name());
    //     }
    //     scene_.delete_root_node(player.scene_node_name());
    // }
}

void Spawn::respawn_all_players() {
    for (const auto& [_, p] : players_.players()) {
        const std::string& node_name = p->scene_node_name();
        if (!node_name.empty()) {
            // Lock guard avoids this error during rendering:
            // "Could not find black node with name ..."
            std::lock_guard lock{ delete_node_mutex_ };
            scene_.delete_root_node(node_name);
            ++ndelete_;
        }
    }
    std::set<std::string> all_teams;
    for (const auto& [_, p] : players_.players()) {
        all_teams.insert(p->team());
    }
    std::set<SpawnPoint*> occupied_spawn_points;
    for (const std::string& team : all_teams) {
        auto sit = spawn_points_.begin();
        auto pit = players_.players().begin();
        for (; sit != spawn_points_.end() && pit != players_.players().end();) {
            if (!sit->team.empty() && (sit->team != team)) {
                ++sit;
                continue;
            }
            if (pit->second->team() != team) {
                ++pit;
                continue;
            }
            if (sit->type != SpawnPointType::SPAWN_LINE) {
                ++sit;
                continue;
            }
            if (occupied_spawn_points.contains(&*sit)) {
                ++sit;
                continue;
            }
            // std::cerr << "Spawning " << pit->second->name() << " with team " << pit->second->team() << std::endl;
            spawn_at_spawn_point(*pit->second, *sit);
            occupied_spawn_points.insert(&*sit);
            ++sit;
            ++pit;
        }
    }
}
