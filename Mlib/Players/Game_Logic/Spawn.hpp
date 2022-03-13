#pragma once
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <vector>

namespace Mlib {

class Bystanders;
struct SpawnPoint;
class Player;
class Players;
class GameLogic;
struct GameLogicConfig;
class Scene;
class SceneNode;
class TeamDeathmatch;
class DeleteNodeMutex;
template <class TData, class TPayload, size_t tndim>
class Bvh;

class Spawn {
    friend Bystanders;
    friend TeamDeathmatch;
    friend GameLogic;
public:
    explicit Spawn(
        Players& players,
        GameLogicConfig& cfg,
        DeleteNodeMutex& delete_node_mutex,
        Scene& scene);
    ~Spawn();
    void set_spawn_points(
        const SceneNode& node,
        const std::list<SpawnPoint>& spawn_points);
    void set_preferred_car_spawner(
        Player& player,
        const std::function<void(const SpawnPoint&)>& preferred_car_spawner);
    void respawn_all_players();
private:
    void spawn_at_spawn_point(
        Player& player,
        const SpawnPoint& sp);
    std::vector<SpawnPoint> spawn_points_;
    std::vector<std::unique_ptr<Bvh<float, const SpawnPoint*, 3>>> spawn_points_bvhs_;
    std::map<const Player*, std::function<void(const SpawnPoint&)>> preferred_car_spawners_;
    Players& players_;
    GameLogicConfig& cfg_;
    DeleteNodeMutex& delete_node_mutex_;
    Scene& scene_;
    size_t nspawns_;
    size_t ndelete_;
};

}
