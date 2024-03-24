#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Memory/Destruction_Observers.hpp>
#include <Mlib/Object.hpp>
#include <Mlib/Physics/Driving_Mode.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Interfaces/External_Force_Provider.hpp>
#include <Mlib/Physics/Interfaces/IPlayer.hpp>
#include <Mlib/Players/Player/Avatar_Movement.hpp>
#include <Mlib/Players/Player/Car_Movement.hpp>
#include <Mlib/Players/Player/Pathfinding_Waypoints.hpp>
#include <Mlib/Players/Player/Playback_Waypoints.hpp>
#include <Mlib/Players/Player/Player_Stats.hpp>
#include <Mlib/Players/Player/Single_Waypoint.hpp>
#include <Mlib/Players/Player/Supply_Depots_Waypoints.hpp>
#include <Mlib/Players/Player/Vehicle_Movement.hpp>
#include <Mlib/Players/Scene_Vehicle/Skills.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <chrono>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>

namespace Mlib {

class RigidBodyVehicle;
class Players;
class Team;
class SceneNode;
class Scene;
class SupplyDepots;
class CollisionQuery;
class AimAt;
class Gun;
enum class DrivingDirection;
enum class WayPointLocation;
class DeleteNodeMutex;
class Bystanders;
class WeaponCycle;
class Inventory;
class Focuses;
class SceneVehicle;
class VehicleSpawner;
class VehicleSpawners;

enum class GameMode {
    RAMMING,
    TEAM_DEATHMATCH,
    RACING,
    BYSTANDER,
};

inline GameMode game_mode_from_string(const std::string& game_mode) {
    if (game_mode == "ramming") {
        return GameMode::RAMMING;
    } if (game_mode == "team_deathmatch") {
        return GameMode::TEAM_DEATHMATCH;
    } else if (game_mode == "racing") {
        return GameMode::RACING;
    } else if (game_mode == "bystander") {
        return GameMode::BYSTANDER;
    } else {
        THROW_OR_ABORT("Unknown game mode: " + game_mode);
    }
}

enum class ExternalsMode;

enum class UnstuckMode {
    OFF,
    REVERSE,
    DELETE
};

inline UnstuckMode unstuck_mode_from_string(const std::string& unstuck_mode) {
    if (unstuck_mode == "off") {
        return UnstuckMode::OFF;
    } else if (unstuck_mode == "reverse") {
        return UnstuckMode::REVERSE;
    } else if (unstuck_mode == "delete") {
        return UnstuckMode::DELETE;
    } else {
        THROW_OR_ABORT("Unknown unstuck mode: " + unstuck_mode);
    }
}

enum class ControlSource;

struct PlayerControlled {
    bool has_aim_at() const;
    AimAt& aim_at();
    DanglingPtr<SceneNode> gun_node;
};

class Player:
    public IPlayer,
    public DestructionObserver<DanglingRef<SceneNode>>,
    public DestructionObserver<const SceneVehicle&>,
    public AdvanceTime,
    public ExternalForceProvider,
    public DanglingBaseClass
{
    friend PathfindingWaypoints;
    friend PlaybackWaypoints;
    friend SingleWaypoint;
    friend CarMovement;
    friend AvatarMovement;
    friend VehicleSpawner;
public:
    Player(
        Scene& scene,
        SupplyDepots& supply_depots,
        const PhysicsEngineConfig& cfg,
        CollisionQuery& collision_query,
        VehicleSpawners& vehicle_spawners,
        Players& players,
        const std::string& name,
        const std::string& team,
        GameMode game_mode,
        UnstuckMode unstuck_mode,
        const DrivingMode& driving_mode,
        DrivingDirection driving_direction,
        DeleteNodeMutex& delete_node_mutex,
        const Focuses& focuses);
    virtual ~Player() override;
    void set_can_drive(ControlSource control_source, bool value);
    void set_can_aim(ControlSource control_source, bool value);
    void set_can_shoot(ControlSource control_source, bool value);
    void set_can_select_best_weapon(ControlSource control_source, bool value);
    void reset_node();
    void set_scene_vehicle(SceneVehicle& pv);
    RigidBodyVehicle& rigid_body();
    const RigidBodyVehicle& rigid_body() const;
    DanglingRef<SceneNode> scene_node();
    DanglingRef<const SceneNode> scene_node() const;
    SceneVehicle* next_scene_vehicle();
    const std::string& scene_node_name() const;
    SceneVehicle& vehicle();
    const SceneVehicle& vehicle() const;
    void set_gun_node(DanglingRef<SceneNode> gun_node);
    void set_pathfinding_waypoints(
        const std::map<WayPointLocation, PointsAndAdjacency<double, 3>>& way_points);
    const std::string& team_name() const;
    Team& team();
    PlayerStats& stats();
    const PlayerStats& stats() const;
    float car_health() const;
    GameMode game_mode() const;
    bool can_see(
        const RigidBodyVehicle& rbi,
        bool only_terrain = false,
        float height_offset = 0,
        float time_offset = 0) const;
    bool can_see(
        const FixedArray<double, 3>& pos,
        bool only_terrain = false,
        float height_offset = 0,
        float time_offset = 0) const;
    bool can_see(
        const SceneVehicle& scene_vehicle,
        bool only_terrain = false,
        float height_offset = 0,
        float time_offset = 0) const;
    bool can_see(
        const Player& player,
        bool only_terrain = false,
        float height_offset = 0,
        float time_offset = 0) const;
    bool is_pedestrian() const;
    bool has_scene_vehicle() const;
    bool has_vehicle_controller() const;
    std::string vehicle_name() const;
    FixedArray<float, 3> gun_direction() const;
    FixedArray<float, 3> punch_angle() const;
    bool has_gun_node() const;
    void trigger_gun();
    bool has_weapon_cycle() const;
    Inventory& inventory();
    const Inventory& inventory() const;
    WeaponCycle& weapon_cycle();
    const WeaponCycle& weapon_cycle() const;
    bool needs_supplies() const;
    size_t nbullets_available() const;
    std::string best_weapon_in_inventory() const;
    void select_next_opponent();
    void select_next_vehicle();
    void append_delete_externals(
        DanglingPtr<SceneNode> node,
        const std::function<void()>& delete_externals);
    void append_dependent_node(std::string node_name);
    void create_externals(ExternalsMode externals_mode);
    ExternalsMode externals_mode() const;
    SingleWaypoint& single_waypoint();
    PathfindingWaypoints& pathfinding_waypoints();
    PlaybackWaypoints& playback_waypoints();

    // IPlayer
    virtual const std::string& name() const override;
    virtual void notify_race_started() override;
    virtual RaceState notify_lap_finished(
        float race_time_seconds,
        const std::string& asset_id,
        const std::vector<FixedArray<float, 3>>& vehicle_colors,
        const std::list<float>& lap_times_seconds,
        const std::list<TrackElement>& track) override;
    virtual void notify_vehicle_destroyed() override;
    virtual void notify_kill(RigidBodyVehicle& rigid_body_vehicle) override;
    virtual void notify_bullet_destroyed(Bullet& bullet) override;
    // DestructionObserver
    virtual void notify_destroyed(DanglingRef<SceneNode> destroyed_object) override;
    virtual void notify_destroyed(const SceneVehicle& destroyed_object) override;
    // AdvanceTime
    virtual void advance_time(float dt) override;
    // ExternalForceProvider
    virtual void increment_external_forces(
        const std::list<RigidBodyVehicle*>& olist,
        bool burn_in,
        const PhysicsEngineConfig& cfg) override;
    
    DestructionObservers<const IPlayer&> destruction_observers;
    VehicleMovement vehicle_movement;
    CarMovement car_movement;
    AvatarMovement avatar_movement;
private:
    void clear_opponent();
    void set_opponent(const Player& opponent);
    void aim_and_shoot();
    void select_best_weapon_in_inventory();
    bool ramming() const;
    bool unstuck();
    const Gun& gun() const;
    Gun& gun();
    Scene& scene_;
    CollisionQuery& collision_query_;
    VehicleSpawners& vehicle_spawners_;
    Players& players_;
    std::string name_;
    std::string team_;
    SceneVehicle* vehicle_;
    PlayerControlled controlled_;
    DanglingPtr<SceneNode> target_scene_node_;
    RigidBodyVehicle* target_rb_;
    PlayerStats stats_;
    GameMode game_mode_;
    std::chrono::steady_clock::time_point stuck_start_;
    std::chrono::steady_clock::time_point unstuck_start_;
    UnstuckMode unstuck_mode_;
    DrivingMode driving_mode_;
    DrivingDirection driving_direction_;
    size_t nunstucked_;
    std::unordered_map<ControlSource, Skills> skills_;
    DeleteNodeMutex& delete_node_mutex_;
    SceneVehicle* next_scene_vehicle_;
    std::multimap<DanglingPtr<const SceneNode>, std::function<void()>> delete_externals_;
    std::map<DanglingPtr<const SceneNode>, std::string> dependent_nodes_;
    ExternalsMode externals_mode_;
    SingleWaypoint single_waypoint_;
    PathfindingWaypoints pathfinding_waypoints_;
    SupplyDepotsWaypoints supply_depots_waypoints_;
    PlaybackWaypoints playback_waypoints_;
    const Focuses& focuses_;
};

};
