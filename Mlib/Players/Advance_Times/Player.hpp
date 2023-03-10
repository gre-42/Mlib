#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
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
class YawPitchLookAtNodes;
class Gun;
enum class DrivingDirection;
enum class WayPointLocation;
class DeleteNodeMutex;
class Bystanders;
class WeaponCycle;
class Inventory;
class Focuses;

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

enum class ExternalsMode {
    PC,
    NPC,
    NONE
};

inline ExternalsMode externals_mode_from_string(const std::string& externals_mode) {
    if (externals_mode == "pc") {
        return ExternalsMode::PC;
    } else if (externals_mode == "npc") {
        return ExternalsMode::NPC;
    } else {
        THROW_OR_ABORT("Unknown externals mode: " + externals_mode);
    }
}

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

struct Skills {
    bool can_drive = false;
    bool can_aim = false;
    bool can_shoot = false;
    bool can_select_best_weapon = false;
};

enum class ControlSource {
    AI,
    USER
};

inline ControlSource control_source_from_string(const std::string& control_source) {
    if (control_source == "ai") {
        return ControlSource::AI;
    } else if (control_source == "user") {
        return ControlSource::USER;
    } else {
        THROW_OR_ABORT("Unknown control source: " + control_source);
    }
}

struct PlayerVehicle {
    std::string scene_node_name;
    SceneNode* scene_node;
    RigidBodyVehicle* rb;
    std::function<void(const std::string&, ExternalsMode, const std::unordered_map<ControlSource, Skills>&)> create_externals;
};

struct PlayerControlled {
    YawPitchLookAtNodes* ypln;
    SceneNode* gun_node;
};

class Player: public Object, public IPlayer, DestructionObserver, public AdvanceTime, public ExternalForceProvider {
    friend PathfindingWaypoints;
    friend PlaybackWaypoints;
    friend SingleWaypoint;
    friend CarMovement;
    friend AvatarMovement;
public:
    Player(
        Scene& scene,
        SupplyDepots& supply_depots,
        const PhysicsEngineConfig& cfg,
        CollisionQuery& collision_query,
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
    void set_rigid_body(const PlayerVehicle& pv);
    RigidBodyVehicle& rigid_body();
    const RigidBodyVehicle& rigid_body() const;
    bool has_scene_node() const;
    SceneNode& scene_node();
    const SceneNode& scene_node() const;
    const SceneNode* next_scene_node() const;
    const std::string& scene_node_name() const;
    const PlayerVehicle& vehicle() const;
    void set_ypln(YawPitchLookAtNodes& ypln, SceneNode* gun_node);
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
        const Player& player,
        bool only_terrain = false,
        float height_offset = 0,
        float time_offset = 0) const;
    void notify_spawn();
    float seconds_since_spawn() const;
    bool spotted_by_vip() const;
    void set_spotted_by_vip();
    bool is_pedestrian() const;
    bool has_rigid_body() const;
    std::string vehicle_name() const;
    FixedArray<float, 3> vehicle_color() const;
    FixedArray<float, 3> gun_direction() const;
    FixedArray<float, 3> punch_angle() const;
    bool has_gun_node() const;
    void trigger_gun();
    bool has_weapon_cycle() const;
    Inventory& inventory();
    const Inventory& inventory() const;
    WeaponCycle& weapon_cycle();
    bool needs_supplies() const;
    size_t nbullets_available() const;
    std::string best_weapon_in_inventory() const;
    void select_next_opponent();
    void select_next_vehicle();
    void set_create_externals(
        const std::function<void(const std::string&, ExternalsMode, const std::unordered_map<ControlSource, Skills>&)>& create_externals);
    void append_delete_externals(
        SceneNode* node,
        const std::function<void()>& delete_externals);
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
        const std::list<float>& lap_times_seconds,
        const std::list<TrackElement>& track) override;
    virtual void notify_vehicle_destroyed() override;
    virtual void notify_kill(RigidBodyVehicle& rigid_body_vehicle) override;
    virtual void notify_bullet_destroyed(Bullet& bullet) override;
    // DestructionObserver
    virtual void notify_destroyed(const Object& destroyed_object) override;
    // AdvanceTime
    virtual void advance_time(float dt) override;
    // ExternalForceProvider
    virtual void increment_external_forces(
        const std::list<RigidBodyVehicle*>& olist,
        bool burn_in,
        const PhysicsEngineConfig& cfg) override;
    
    DestructionObservers destruction_observers;
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
    Players& players_;
    std::string name_;
    std::string team_;
    PlayerVehicle vehicle_;
    PlayerControlled controlled_;
    SceneNode* target_scene_node_;
    RigidBodyVehicle* target_rb_;
    PlayerStats stats_;
    GameMode game_mode_;
    std::chrono::time_point<std::chrono::steady_clock> stuck_start_;
    std::chrono::time_point<std::chrono::steady_clock> unstuck_start_;
    UnstuckMode unstuck_mode_;
    DrivingMode driving_mode_;
    DrivingDirection driving_direction_;
    std::chrono::time_point<std::chrono::steady_clock> spawn_time_;
    bool spotted_by_vip_;
    size_t nunstucked_;
    std::unordered_map<ControlSource, Skills> skills_;
    DeleteNodeMutex& delete_node_mutex_;
    SceneNode* next_scene_node_;
    std::multimap<SceneNode*, std::function<void()>> delete_externals_;
    ExternalsMode externals_mode_;
    SingleWaypoint single_waypoint_;
    PathfindingWaypoints pathfinding_waypoints_;
    SupplyDepotsWaypoints supply_depots_waypoints_;
    PlaybackWaypoints playback_waypoints_;
    const Focuses& focuses_;
};

};
