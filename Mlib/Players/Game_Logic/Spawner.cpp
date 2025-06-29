#include "Spawner.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Object.hpp>
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
#include <Mlib/Scene_Graph/Spawn_Arguments.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

#ifndef __clang__
#include <ranges>
#endif

using namespace Mlib;

Spawner::Spawner(
    VehicleSpawners& vehicle_spawners,
    Players& players,
    GameLogicConfig& cfg,
    DeleteNodeMutex& delete_node_mutex,
    Scene& scene)
    : vehicle_spawners_{ vehicle_spawners }
    , players_{ players }
    , cfg_{ cfg }
    , delete_node_mutex_{ delete_node_mutex }
    , scene_{ scene }
{}

Spawner::~Spawner() = default;

void Spawner::set_spawn_points(
    const TransformationMatrix<SceneDir, ScenePos, 3>& absolute_model_matrix,
    const std::list<SpawnPoint>& spawn_points)
{
    spawn_points_.clear();
    spawn_points_.reserve(spawn_points.size());
    FixedArray<SceneDir, 3, 3> R = absolute_model_matrix.R / absolute_model_matrix.get_scale();
    size_t nsubs = cfg_.spawn_points_nsubdivisions;
    spawn_points_bvh_split_.resize(nsubs);
    for (size_t i = 0; i < nsubs; ++i) {
        spawn_points_bvh_split_[i].reset(new Bvh<CompressedScenePos, 3, const SpawnPoint*>(fixed_full<CompressedScenePos, 3>((CompressedScenePos)10.f), 10));
    }
    spawn_points_bvh_singular_.reset(new Bvh<CompressedScenePos, 3, const SpawnPoint*>(fixed_full<CompressedScenePos, 3>((CompressedScenePos)10.f), 10));
    for (const auto& [i, sp] : enumerate(spawn_points)) {
        SpawnPoint sp2 = sp;
        sp2.trafo.t = absolute_model_matrix.transform(funpack(sp.trafo.t)).casted<CompressedScenePos>();
        // The spawn rotation is relative to the surface, so use R R1 R^T.
        sp2.trafo.R = dot2d(dot2d(R, sp.trafo.R), R.T());
        const auto* spb = &spawn_points_.emplace_back(sp2);
        spawn_points_bvh_split_[i % nsubs]->insert(AxisAlignedBoundingBox<CompressedScenePos, 3>::from_point(sp2.trafo.t), spb);
        spawn_points_bvh_singular_->insert(AxisAlignedBoundingBox<CompressedScenePos, 3>::from_point(sp2.trafo.t), spb);
    }
}

bool Spawner::can_spawn_at_spawn_point(
    VehicleSpawner& spawner,
    const TransformationMatrix<SceneDir, CompressedScenePos, 3>& sp) const
{
    // TimeGuard time_guard{"spawn", "spawn"};
    // std::scoped_lock lock{ delete_node_mutex_ };
    // TimeGuard time_guard2{"spawn2", "spawn2"};
    // auto start = std::chrono::steady_clock::now();
    return spawner.try_spawn(sp, { SpawnAction::DRY_RUN });
}

bool Spawner::try_spawn_at_spawn_point(
    VehicleSpawner& spawner,
    const TransformationMatrix<SceneDir, CompressedScenePos, 3>& sp)
{
    // TimeGuard time_guard{"spawn", "spawn"};
    // std::scoped_lock lock{ delete_node_mutex_ };
    // TimeGuard time_guard2{"spawn2", "spawn2"};
    // auto start = std::chrono::steady_clock::now();
    bool success = spawner.try_spawn(sp, { SpawnAction::DO_IT });
    // lerr() << "Spawner time " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<ScenePos>(std::chrono::steady_clock::now() - start)).count();
    ++ntry_spawns_;
    return success;
    // while (true) {
    //     std::scoped_lock lock{ delete_node_mutex_ };
    //     spawn_macro->second(sp2);
    //     if (player.scene_node_name().empty()) {
    //         THROW_OR_ABORT("After spawning, scene node name empty for player " + player.name());
    //     }
    //     scene_.delete_root_node(player.scene_node_name());
    // }
}

void Spawner::respawn_all_players() {
    for (auto& [_, p] : vehicle_spawners_.spawners()) {
        if (!p->has_scene_vehicle()) {
            continue;
        }
        while (p->has_scene_vehicle()) {
            auto node_name = p->get_primary_scene_vehicle()->scene_node_name();
            scene_.delete_root_node(node_name);
        }
        ++ndelete_;
    }
    std::set<SpawnPoint*> occupied_spawn_points;
    for (const auto& [name, spawner] : vehicle_spawners_.spawners()) {
        auto shuffled_spawn_pts = shuffled_spawn_points();
        for (const auto& sp : shuffled_spawn_pts) {
            if (!sp->team.empty() && (sp->team != spawner->get_team_name())) {
                continue;
            }
            if (spawner->get_group_name() != sp->group) {
                continue;
            }
            if (sp->type != SpawnPointType::SPAWN_LINE) {
                continue;
            }
            if (occupied_spawn_points.contains(sp)) {
                continue;
            }
            // lerr() << "Spawning \"" << pit->first << "\" with team \"" << pit->second->get_team_name() << '"'";
            if (!try_spawn_at_spawn_point(*spawner, sp->trafo)) {
                THROW_OR_ABORT("Could not spawn \"" + name + "\" with team \"" + spawner->get_team_name() + '"');
            }
            occupied_spawn_points.insert(sp);
        }
    }
}

bool Spawner::try_spawn_player_during_match(VehicleSpawner& spawner) {
    std::set<const SpawnPoint*> occupied_spawn_points;
    for (const auto& [_, p] : vehicle_spawners_.spawners()) {
        if (p->has_scene_vehicle()) {
            for (const auto& v : p->get_scene_vehicles()) {
                auto pos = v->rb()->rbp_.abs_position();
                spawn_points_bvh_singular_->visit(
                    AxisAlignedBoundingBox<CompressedScenePos, 3>::from_center_and_radius(
                        pos.casted<CompressedScenePos>(),
                        cfg_.r_occupied_spawn_point),
                    [&](const SpawnPoint* sp) {
                        if (sum(squared(pos - funpack(sp->trafo.t))) < squared(cfg_.r_occupied_spawn_point)) {
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
        if (spawner.get_group_name() != sp->group) {
            continue;
        }
        if (sp->type != SpawnPointType::SPAWN_LINE) {
            continue;
        }
        if (occupied_spawn_points.contains(sp)) {
            continue;
        }
        if (try_spawn_at_spawn_point(spawner, sp->trafo)) {
            return true;
        }
    }
    return false;
}

std::vector<SpawnPoint*> Spawner::shuffled_spawn_points() {
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
