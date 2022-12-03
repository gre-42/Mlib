#include "Spawn.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Game_Logic/Game_Logic_Config.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>

#ifndef __clang__
#include <ranges>
#endif

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

Spawn::~Spawn() = default;

void Spawn::set_spawn_points(const SceneNode& node, const std::list<SpawnPoint>& spawn_points) {
    spawn_points_.clear();
    spawn_points_.reserve(spawn_points.size());
    TransformationMatrix tm{node.absolute_model_matrix()};
    FixedArray<float, 3, 3> R = tm.R() / node.scale();
    size_t nsubs = cfg_.spawn_points_nsubdivisions;
    spawn_points_bvh_split_.resize(nsubs);
    for (size_t i = 0; i < nsubs; ++i) {
        spawn_points_bvh_split_[i].reset(new Bvh<double, const SpawnPoint*, 3>(FixedArray<double, 3>{100., 100., 100.}, 10));
    }
    spawn_points_bvh_singular_.reset(new Bvh<double, const SpawnPoint*, 3>(FixedArray<double, 3>{100., 100., 100.}, 10));
    {
        size_t i = 0;
        for (const auto& sp : spawn_points) {
            SpawnPoint sp2 = sp;
            sp2.position = tm.transform(sp.position);
            sp2.rotation = matrix_2_tait_bryan_angles(dot2d(dot2d(R, tait_bryan_angles_2_matrix(sp.rotation)), R.T()));
            spawn_points_.push_back(sp2);
            spawn_points_bvh_split_[i]->insert(sp2.position, &spawn_points_.back());
            spawn_points_bvh_singular_->insert(sp2.position, &spawn_points_.back());
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
        all_teams.insert(p->team_name());
    }
    std::set<SpawnPoint*> occupied_spawn_points;
    auto shuffled_spawn_pts = shuffled_spawn_points();
    for (const std::string& team : all_teams) {
        auto sit = shuffled_spawn_pts.begin();
        auto pit = players_.players().begin();
        for (; sit != shuffled_spawn_pts.end() && pit != players_.players().end();) {
            auto& sp = (**sit);
            if (!sp.team.empty() && (sp.team != team)) {
                ++sit;
                continue;
            }
            if (pit->second->team_name() != team) {
                ++pit;
                continue;
            }
            if (sp.type != SpawnPointType::SPAWN_LINE) {
                ++sit;
                continue;
            }
            if (occupied_spawn_points.contains(&sp)) {
                ++sit;
                continue;
            }
            // std::cerr << "Spawning " << pit->second->name() << " with team " << pit->second->team_name() << std::endl;
            spawn_at_spawn_point(*pit->second, sp);
            occupied_spawn_points.insert(&sp);
            ++sit;
            ++pit;
        }
    }
}

void Spawn::spawn_player_during_match(Player& player) {
    std::set<const SpawnPoint*> occupied_spawn_points;
    for (const auto& [_, p] : players_.players()) {
        if (p->has_rigid_body()) {
            auto pos = p->rigid_body().rbi_.abs_position();
            spawn_points_bvh_singular_->visit(
                AxisAlignedBoundingBox<double, 3>(pos, cfg_.r_occupied_spawn_point),
                [&](const SpawnPoint* sp) {
                    if (sum(squared(pos - sp->position)) < squared(cfg_.r_occupied_spawn_point)) {
                        occupied_spawn_points.insert(sp);
                    }
                    return true;
                });
        }
    }
    auto shuffled_spawn_pts = shuffled_spawn_points();
    for (const auto& sp : shuffled_spawn_pts) {
        if (!sp->team.empty() && (sp->team != player.team_name())) {
            continue;
        }
        if (sp->type != SpawnPointType::SPAWN_LINE) {
            continue;
        }
        if (occupied_spawn_points.contains(sp)) {
            continue;
        }
        spawn_at_spawn_point(player, *sp);
        break;
    }
}

std::vector<SpawnPoint*> Spawn::shuffled_spawn_points() {
#ifdef __clang__
    std::vector<SpawnPoint*> result;
    result.reserve(spawn_points_.size());
    for (auto& p : spawn_points_) {
        result.push_back(&p);
    }
#else
    auto range = spawn_points_ | std::views::transform([](auto& p){return &p;});
    std::vector<SpawnPoint*> result(range.begin(), range.end());
#endif
    if (!getenv_default_bool("NO_SHUFFLE_SPAWN_POINTS", false)) {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(result.begin(), result.end(), g);
    }    
    return result;
}
