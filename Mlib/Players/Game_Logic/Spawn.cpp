#include "Spawn.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Game_Logic/Game_Logic_Config.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

#ifndef __clang__
#include <ranges>
#endif

using namespace Mlib;

Spawn::Spawn(
    VehicleSpawners& vehicle_spawners,
    Players& players,
    GameLogicConfig& cfg,
    DeleteNodeMutex& delete_node_mutex,
    Scene& scene)
: vehicle_spawners_{ vehicle_spawners },
  players_{ players },
  cfg_{ cfg },
  delete_node_mutex_{ delete_node_mutex },
  scene_{ scene }
{}

Spawn::~Spawn() = default;

void Spawn::set_spawn_points(DanglingRef<const SceneNode> node, const std::list<SpawnPoint>& spawn_points) {
    spawn_points_.clear();
    spawn_points_.reserve(spawn_points.size());
    TransformationMatrix tm{node->absolute_model_matrix()};
    FixedArray<float, 3, 3> R = tm.R() / node->scale();
    size_t nsubs = cfg_.spawn_points_nsubdivisions;
    spawn_points_bvh_split_.resize(nsubs);
    for (size_t i = 0; i < nsubs; ++i) {
        spawn_points_bvh_split_[i].reset(new Bvh<double, const SpawnPoint*, 3>(FixedArray<double, 3>{10., 10., 10.}, 10));
    }
    spawn_points_bvh_singular_.reset(new Bvh<double, const SpawnPoint*, 3>(FixedArray<double, 3>{10., 10., 10.}, 10));
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

void Spawn::spawn_at_spawn_point(
    VehicleSpawner& spawner,
    const SpawnPoint& sp)
{
    // TimeGuard time_guard{"spawn", "spawn"};
    // std::scoped_lock lock{ delete_node_mutex_ };
    // TimeGuard time_guard2{"spawn2", "spawn2"};
    // auto start = std::chrono::steady_clock::now();
    spawner.spawn(sp, cfg_.spawn_y_offset);
    // std::cerr << "Spawn time " << 1000 * std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count() << std::endl;
    ++nspawns_;
    // while (true) {
    //     std::scoped_lock lock{ delete_node_mutex_ };
    //     spawn_macro->second(sp2);
    //     if (player.scene_node_name().empty()) {
    //         THROW_OR_ABORT("After spawning, scene node name empty for player " + player.name());
    //     }
    //     scene_.delete_root_node(player.scene_node_name());
    // }
}

void Spawn::respawn_all_players() {
    for (auto& [_, p] : vehicle_spawners_.spawners()) {
        if (!p->has_scene_vehicle()) {
            continue;
        }
        while (p->has_scene_vehicle()) {
            std::string node_name = p->get_primary_scene_vehicle().scene_node_name();
            // Lock guard avoids this error during rendering:
            // "Could not find black node with name ..."
            std::scoped_lock lock{ delete_node_mutex_ };
            scene_.delete_root_node(node_name);
        }
        ++ndelete_;
    }
    std::set<std::string> all_teams;
    for (const auto& [_, p] : players_.players()) {
        all_teams.insert(p->team_name());
    }
    std::set<SpawnPoint*> occupied_spawn_points;
    auto shuffled_spawn_pts = shuffled_spawn_points();
    for (const std::string& team : all_teams) {
        auto sit = shuffled_spawn_pts.begin();
        auto pit = vehicle_spawners_.spawners().begin();
        for (; sit != shuffled_spawn_pts.end() && pit != vehicle_spawners_.spawners().end();) {
            auto& sp = (**sit);
            if (!sp.team.empty() && (sp.team != team)) {
                ++sit;
                continue;
            }
            if (pit->second->get_team_name() != team) {
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

void Spawn::spawn_player_during_match(VehicleSpawner& spawner) {
    std::set<const SpawnPoint*> occupied_spawn_points;
    for (auto& [_, p] : vehicle_spawners_.spawners()) {
        if (p->has_scene_vehicle()) {
            for (const auto& v : p->get_scene_vehicles()) {
                auto pos = v->rb().rbp_.abs_position();
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
    }
    auto shuffled_spawn_pts = shuffled_spawn_points();
    for (const auto& sp : shuffled_spawn_pts) {
        if (!sp->team.empty() && (sp->team != spawner.get_team_name())) {
            continue;
        }
        if (sp->type != SpawnPointType::SPAWN_LINE) {
            continue;
        }
        if (occupied_spawn_points.contains(sp)) {
            continue;
        }
        spawn_at_spawn_point(spawner, *sp);
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
