#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Driving_Mode.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Interfaces/External_Force_Provider.hpp>
#include <Mlib/Physics/Interfaces/IPlayer.hpp>
#include <chrono>
#include <list>
#include <map>
#include <mutex>
#include <string>

namespace Mlib {

class RigidBodyVehicle;
struct RigidBodyIntegrator;
class Players;
class SceneNode;
class Scene;
class CollisionQuery;
class YawPitchLookAtNodes;
class Gun;
template <class TData, class TPayload, size_t tndim>
class Bvh;
enum class DrivingDirection;
enum class WayPointLocation;
class DeleteNodeMutex;
class PodBotPlayer;

struct PlayerStats {
    size_t nwins = 0;
};

enum class GameMode {
    RAMMING,
    RACING,
    BYSTANDER,
    POD_BOT_NPC,
    POD_BOT_PC
};

inline GameMode game_mode_from_string(const std::string& game_mode) {
    if (game_mode == "ramming") {
        return GameMode::RAMMING;
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

class Player: public IPlayer, DestructionObserver, public AdvanceTime, public ExternalForceProvider {
    friend PodBotPlayer;
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
    void set_rigid_body(const std::string& scene_node_name, SceneNode& scene_node, RigidBodyVehicle& rb);
    const RigidBodyVehicle& rigid_body() const;
    const std::string& scene_node_name() const;
    void set_ypln(YawPitchLookAtNodes& ypln, SceneNode* gun_node);
    void set_surface_power(float forward, float backward);
    void set_tire_angle_y(size_t tire_id, float angle_left, float angle_right);
    void set_angular_velocity(float angle_left, float angle_right);
    void draw_waypoint_history(const std::string& filename) const;
    void set_waypoint(const FixedArray<float, 3>& waypoint, size_t waypoint_id);
    void set_waypoint(const FixedArray<float, 3>& waypoint);
    void set_waypoint(size_t waypoint_id);
    void set_waypoints(
        const SceneNode& node,
        const std::map<WayPointLocation, PointsAndAdjacency<float, 3>>& all_waypoints);
    const std::string& team() const;
    PlayerStats& stats();
    const PlayerStats& stats() const;
    float car_health() const;
    GameMode game_mode() const;
    bool can_see(
        const RigidBodyIntegrator& rbi,
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
    bool has_waypoints() const;
    bool is_pedestrian() const;
    bool has_rigid_body() const;
    std::string vehicle_name() const;
    FixedArray<float, 3> gun_direction() const;
    FixedArray<float, 3> punch_angle() const;
    void run_move(
        float yaw,
        float pitch,
        float forwardmove,
        float sidemove);
    void trigger_gun();

    virtual const std::string& name() const override;
    virtual void notify_lap_time(
        float lap_time,
        const std::list<TrackElement>& track) override;
    virtual void notify_destroyed(void* destroyed_object) override;
    virtual void advance_time(float dt) override;
    virtual void increment_external_forces(const std::list<std::shared_ptr<RigidBodyVehicle>>& olist, bool burn_in, const PhysicsEngineConfig& cfg) override;
private:
    void select_opponent();
    void aim_and_shoot();
    void move_to_waypoint();
    void select_next_waypoint();
    bool ramming() const;
    bool unstuck();
    void step_on_breaks();
    void drive_forward();
    void drive_backwards();
    void roll_tires();
    void steer_left_full();
    void steer_right_full();
    void steer_left_partial(float angle);
    void steer_right_partial(float angle);
    const PointsAndAdjacency<float, 3>& waypoints() const;
    const Gun* gun() const;
    Gun* gun();
    Scene& scene_;
    CollisionQuery& collision_query_;
    Players& players_;
    std::string name_;
    std::string team_;
    std::string scene_node_name_;
    SceneNode* scene_node_;
    SceneNode* target_scene_node_;
    RigidBodyVehicle* rb_;
    RigidBodyIntegrator* target_rbi_;
    YawPitchLookAtNodes* ypln_;
    SceneNode* gun_node_;
    float surface_power_forward_;
    float surface_power_backward_;
    std::map<size_t, float> tire_angles_left_;
    std::map<size_t, float> tire_angles_right_;
    float angular_velocity_left_;
    float angular_velocity_right_;
    FixedArray<float, 3> waypoint_;
    std::list<FixedArray<float, 3>> waypoint_history_;
    std::map<WayPointLocation, PointsAndAdjacency<float, 3>> all_waypoints_;
    std::map<WayPointLocation, Bvh<float, size_t, 3>> all_waypoints_bvh_;
    std::vector<std::chrono::time_point<std::chrono::steady_clock>> last_visited_;
    size_t waypoint_id_;
    bool waypoint_reached_;
    size_t nwaypoints_reached_;
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
    bool record_waypoints_;
    Skills skills_;
    std::unique_ptr<PodBotPlayer> pod_bot_player_;
    DeleteNodeMutex& delete_node_mutex_;
};

};
