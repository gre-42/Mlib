#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Driving_Mode.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Interfaces/External_Force_Provider.hpp>
#include <Mlib/Physics/Interfaces/IPlayer.hpp>
#include <Mlib/Players/Player/Pathfinding_Waypoints.hpp>
#include <Mlib/Players/Player/Playback_Waypoints.hpp>
#include <Mlib/Players/Player/Single_Waypoint.hpp>
#include <Mlib/Signal/Pid_Controller.hpp>
#include <chrono>
#include <list>
#include <map>
#include <mutex>
#include <set>
#include <string>

namespace Mlib {

class RigidBodyVehicle;
class Players;
class SceneNode;
class Scene;
class CollisionQuery;
class YawPitchLookAtNodes;
class Gun;
enum class DrivingDirection;
enum class WayPointLocation;
class DeleteNodeMutex;
class PodBotPlayer;
template <class TData, size_t n>
class TransformationMatrix;
class Bystanders;

struct PlayerStats {
    size_t nwins = 0;
};

enum class GameMode {
    RAMMING,
    TEAM_DEATHMATCH,
    RACING,
    BYSTANDER,
    POD_BOT_NPC,
    POD_BOT_PC
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
    } else if (game_mode == "pod_bot_npc") {
        return GameMode::POD_BOT_NPC;
    } else if (game_mode == "pod_bot_pc") {
        return GameMode::POD_BOT_PC;
    } else {
        throw std::runtime_error("Unknown game mode: " + game_mode);
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
        throw std::runtime_error("Unknown unstuck mode: " + unstuck_mode);
    }
}

struct Skills {
    bool can_drive = true;
    bool can_aim = true;
    bool can_shoot = true;
};

struct PlayerVehicle {
    std::string scene_node_name;
    SceneNode* scene_node;
    RigidBodyVehicle* rb;
    YawPitchLookAtNodes* ypln;
    SceneNode* gun_node;
    std::function<void(const std::string&)> create_externals;
};

class Player: public IPlayer, DestructionObserver, public AdvanceTime, public ExternalForceProvider {
    friend PodBotPlayer;
    friend PathfindingWaypoints;
    friend PlaybackWaypoints;
    friend SingleWaypoint;
public:
    Player(
        Scene& scene,
        CollisionQuery& collision_query,
        Players& players,
        const std::string& name,
        const std::string& team,
        GameMode game_mode,
        UnstuckMode unstuck_mode,
        const DrivingMode& driving_mode,
        DrivingDirection driving_direction,
        DeleteNodeMutex& delete_node_mutex);
    virtual ~Player() override;
    void set_can_drive(bool value);
    void set_can_aim(bool value);
    void set_can_shoot(bool value);
    void reset_node();
    void set_rigid_body(const PlayerVehicle& pv);
    const RigidBodyVehicle& rigid_body() const;
    bool has_scene_node() const;
    SceneNode& scene_node();
    const SceneNode& scene_node() const;
    const SceneNode* next_scene_node() const;
    const std::string& scene_node_name() const;
    const PlayerVehicle& vehicle() const;
    void set_ypln(YawPitchLookAtNodes& ypln, SceneNode* gun_node);
    void set_vehicle_control_parameters(
        float surface_power_forward,
        float surface_power_backward,
        float max_tire_angle,
        const PidController<float, float>& tire_angle_pid);
    void set_waypoints(
        const TransformationMatrix<double, 3>& inverse_geographic_mapping,
        const std::string& playback_filename);
    const std::string& team() const;
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
        const FixedArray<float, 3>& pos,
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
    void run_move(
        float yaw,
        float pitch,
        float forwardmove,
        float sidemove);
    void trigger_gun();
    void select_next_opponent();
    void select_next_vehicle();
    void set_create_externals(const std::function<void(const std::string&)>& create_externals);
    void append_delete_externals(
        SceneNode* node,
        const std::function<void()>& delete_externals);
    void create_externals();
    bool externals_created() const;
    SingleWaypoint& single_waypoint();
    PathfindingWaypoints& pathfinding_waypoints();
    PlaybackWaypoints& playback_waypoints();

    // IPlayer
    virtual const std::string& name() const override;
    virtual void notify_lap_time(
        float lap_time,
        const std::list<TrackElement>& track) override;
    virtual void notify_vehicle_destroyed() override;
    // DestructionObserver
    virtual void notify_destroyed(void* destroyed_object) override;
    // AdvanceTime
    virtual void advance_time(float dt) override;
    // ExternalForceProvider
    virtual void increment_external_forces(
        const std::list<std::shared_ptr<RigidBodyVehicle>>& olist,
        bool burn_in,
        const PhysicsEngineConfig& cfg) override;
private:
    void aim_and_shoot();
    bool ramming() const;
    bool unstuck();
    void step_on_brakes();
    void drive_forward();
    void drive_backwards();
    void roll_tires();
    void steer(float angle);
    void steer_left_full();
    void steer_right_full();
    void steer_left_partial(float angle);
    void steer_right_partial(float angle);
    const Gun* gun() const;
    Gun* gun();
    Scene& scene_;
    CollisionQuery& collision_query_;
    Players& players_;
    std::string name_;
    std::string team_;
    PlayerVehicle vehicle_;
    SceneNode* target_scene_node_;
    RigidBodyVehicle* target_rb_;
    float surface_power_forward_;
    float surface_power_backward_;
    float max_tire_angle_;
    PidController<float, float> tire_angle_pid_;
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
    Skills skills_;
    std::unique_ptr<PodBotPlayer> pod_bot_player_;
    DeleteNodeMutex& delete_node_mutex_;
    SceneNode* next_scene_node_;
    std::multimap<SceneNode*, std::function<void()>> delete_externals_;
    bool externals_created_;
    SingleWaypoint single_waypoint_;
    PathfindingWaypoints pathfinding_waypoints_;
    PlaybackWaypoints playback_waypoints_;
};

};
