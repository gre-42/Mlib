#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Driving_Mode.hpp>
#include <Mlib/Physics/Interfaces/Advance_Time.hpp>
#include <Mlib/Physics/Interfaces/External_Force_Provider.hpp>
#include <chrono>
#include <list>
#include <map>
#include <mutex>
#include <string>

namespace Mlib {

struct RigidBody;
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

struct PlayerStats {
    size_t nwins = 0;
};

enum class GameMode {
    RAMMING,
    RACING,
    BYSTANDER
};

inline GameMode game_mode_from_string(const std::string& game_mode) {
    if (game_mode == "ramming") {
        return GameMode::RAMMING;
    } else if (game_mode == "racing") {
        return GameMode::RACING;
    } else if (game_mode == "bystander") {
        return GameMode::BYSTANDER;
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

class Player: public DestructionObserver, public AdvanceTime, public ExternalForceProvider {
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
        std::recursive_mutex& mutex);
    ~Player();
    void set_rigid_body(const std::string& scene_node_name, SceneNode& scene_node, RigidBody& rb);
    const std::string& scene_node_name() const;
    void set_ypln(YawPitchLookAtNodes& ypln, Gun* gun);
    void set_surface_power(float forward, float backward);
    void set_tire_angle_y(size_t tire_id, float angle_left, float angle_right);
    void set_angular_velocity(float angle_left, float angle_right);
    void draw_waypoint_history(const std::string& filename) const;
    void set_waypoint(const FixedArray<float, 2>& waypoint, size_t waypoint_id);
    void set_waypoint(const FixedArray<float, 2>& waypoint);
    void set_waypoint(size_t waypoint_id);
    void set_waypoints(
        const SceneNode& node,
        const std::map<WayPointLocation, PointsAndAdjacency<float, 2>>& all_waypoints);
    const std::string& name() const;
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
    bool spotted() const;
    void set_spotted();
    bool has_waypoints() const;
    bool is_pedestrian() const;

    virtual void notify_destroyed(void* destroyed_object) override;
    virtual void advance_time(float dt) override;
    virtual void increment_external_forces(const std::list<std::shared_ptr<RigidBody>>& olist, bool burn_in, const PhysicsEngineConfig& cfg) override;
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
    void roll();
    void steer_left_full();
    void steer_right_full();
    void steer_left_partial(float angle);
    void steer_right_partial(float angle);
    const PointsAndAdjacency<float, 2>& waypoints() const;
    Scene& scene_;
    CollisionQuery& collision_query_;
    Players& players_;
    std::string name_;
    std::string team_;
    std::string scene_node_name_;
    SceneNode* scene_node_;
    SceneNode* target_scene_node_;
    RigidBody* rb_;
    RigidBodyIntegrator* target_rbi_;
    YawPitchLookAtNodes* ypln_;
    Gun* gun_;
    float surface_power_forward_;
    float surface_power_backward_;
    std::map<size_t, float> tire_angles_left_;
    std::map<size_t, float> tire_angles_right_;
    float angular_velocity_left_;
    float angular_velocity_right_;
    FixedArray<float, 2> waypoint_;
    std::list<FixedArray<float, 2>> waypoint_history_;
    std::map<WayPointLocation, PointsAndAdjacency<float, 2>> all_waypoints_;
    std::map<WayPointLocation, Bvh<float, size_t, 2>> all_waypoints_bvh_;
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
    std::recursive_mutex& mutex_;
    std::chrono::time_point<std::chrono::steady_clock> spawn_time_;
    bool spotted_;
    size_t nunstucked_;
    bool record_waypoints_;
};

};
