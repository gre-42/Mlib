#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Geometry/Mesh/Point_And_Flags.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Memory/Destruction_Observers.hpp>
#include <Mlib/Object.hpp>
#include <Mlib/Physics/Ai/Skill_Map.hpp>
#include <Mlib/Physics/Ai/Skills.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Physics/Interfaces/IExternal_Force_Provider.hpp>
#include <Mlib/Physics/Interfaces/IPlayer.hpp>
#include <Mlib/Players/Player/Avatar_Movement.hpp>
#include <Mlib/Players/Player/Car_Movement.hpp>
#include <Mlib/Players/Player/Pathfinding_Waypoints.hpp>
#include <Mlib/Players/Player/Playback_Waypoints.hpp>
#include <Mlib/Players/Player/Player_Stats.hpp>
#include <Mlib/Players/Player/Single_Waypoint.hpp>
#include <Mlib/Players/Player/Supply_Depots_Waypoints.hpp>
#include <Mlib/Players/Player/Vehicle_Movement.hpp>
#include <Mlib/Players/Scene_Vehicle/Internals_Mode.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <chrono>
#include <list>
#include <mutex>
#include <optional>
#include <string>

namespace Mlib {

class Spawner;
class RigidBodyVehicle;
class Players;
class Team;
class SceneNode;
class Scene;
class SupplyDepots;
class CollisionQuery;
class AimAt;
class Gun;
class Navigate;
enum class DrivingDirection;
enum class JoinedWayPointSandbox;
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
    RALLY,
    BYSTANDER,
};

GameMode game_mode_from_string(const std::string& game_mode);

enum class ExternalsMode;

enum class UnstuckMode {
    OFF,
    REVERSE,
    DELETE
};

UnstuckMode unstuck_mode_from_string(const std::string& unstuck_mode);

enum class OpponentSelectionStrategy {
    KEEP,
    NEXT,
    BEST
};

enum class ControlSource;

struct PlayerControlled {
    bool has_aim_at() const;
    AimAt& aim_at();
    DanglingPtr<SceneNode> gun_node;
};

class Player final:
    public IPlayer,
    public DestructionObserver<SceneNode&>,
    public DestructionObserver<const SceneVehicle&>,
    public DestructionObserver<const RigidBodyVehicle&>,
    public IAdvanceTime,
    public IExternalForceProvider,
    public virtual DanglingBaseClass
{
    friend PathfindingWaypoints;
    friend PlaybackWaypoints;
    friend SingleWaypoint;
    friend CarMovement;
    friend AvatarMovement;
    friend VehicleSpawner;
    Player(const Player&) = delete;
    Player& operator = (const Player&) = delete;
public:
    Player(
        Scene& scene,
        SupplyDepots& supply_depots,
        const Navigate& navigate,
        Spawner& spawner,
        const PhysicsEngineConfig& cfg,
        CollisionQuery& collision_query,
        VehicleSpawners& vehicle_spawners,
        Players& players,
        std::string id,
        std::string team,
        GameMode game_mode,
        UnstuckMode unstuck_mode,
        std::string behavior,
        DrivingDirection driving_direction,
        DeleteNodeMutex& delete_node_mutex,
        const Focuses& focuses);
    virtual ~Player() override;
    void set_can_drive(ControlSource control_source, bool value);
    void set_can_aim(ControlSource control_source, bool value);
    void set_can_shoot(ControlSource control_source, bool value);
    void set_can_select_weapon(ControlSource control_source, bool value);
    void set_can_select_opponent(ControlSource control_source, bool value);
    void set_select_opponent_hysteresis_factor(ScenePos factor);
    void reset_node();
    void set_vehicle_spawner(VehicleSpawner& spawner, const std::string& desired_role);
    RigidBodyVehicle& rigid_body();
    const RigidBodyVehicle& rigid_body() const;
    DanglingRef<SceneNode> scene_node();
    DanglingRef<const SceneNode> scene_node() const;
    bool scene_node_scheduled_for_deletion() const;
    VehicleSpawner* next_scene_vehicle();
    const std::string& next_role() const;
    const std::string& scene_node_name() const;
    SceneVehicle& vehicle();
    const SceneVehicle& vehicle() const;
    VehicleSpawner& vehicle_spawner();
    const VehicleSpawner& vehicle_spawner() const;
    void set_gun_node(DanglingRef<SceneNode> gun_node);
    void change_gun_node(DanglingPtr<SceneNode> gun_node);
    bool has_way_points() const;
    void set_way_point_location_filter(JoinedWayPointSandbox filter);
    const std::string& team_name() const;
    DanglingBaseClassRef<Team> team();
    PlayerStats& stats();
    const PlayerStats& stats() const;
    float car_health() const;
    GameMode game_mode() const;
    bool can_see(
        const RigidBodyVehicle& rbi,
        bool only_terrain = false,
        ScenePos height_offset = 0,
        float time_offset = 0) const;
    bool can_see(
        const FixedArray<ScenePos, 3>& pos,
        bool only_terrain = false,
        ScenePos height_offset = 0,
        float time_offset = 0) const;
    bool can_see(
        const SceneVehicle& scene_vehicle,
        bool only_terrain = false,
        ScenePos height_offset = 0,
        float time_offset = 0) const;
    bool can_see(
        const Player& player,
        bool only_terrain = false,
        ScenePos height_offset = 0,
        float time_offset = 0) const;
    bool is_pedestrian() const;
    bool is_parking() const;
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
    std::optional<std::string> best_weapon_in_inventory() const;
    void select_opponent(OpponentSelectionStrategy strategy);
    void select_next_vehicle();
    void request_reset_vehicle_to_last_checkpoint();
    void append_dependent_node(std::string node_name);
    void create_vehicle_externals(ExternalsMode externals_mode);
    void create_vehicle_internals(const InternalsMode& internals_mode);
    void set_role(const std::string& ui);
    void change_role();
    const Skills& skills(ControlSource control_source) const;
    Players& players();
    bool ramming() const;
    DanglingPtr<SceneNode> target_scene_node() const;
    const RigidBodyVehicle* target_rb() const;
    void set_behavior(
        float stuck_velocity,
        float stuck_duration,
        float unstuck_duration,
        JoinedWayPointSandbox joined_way_point_sandbox);
    DrivingDirection driving_direction() const;
    ExternalsMode externals_mode() const;
    const InternalsMode& internals_mode() const;
    SingleWaypoint& single_waypoint();
    PathfindingWaypoints& pathfinding_waypoints();
    PlaybackWaypoints& playback_waypoints();

    // IPlayer
    virtual std::string id() const override;
    virtual std::string title() const override;
    virtual std::optional<std::string> target_id() const override;
    virtual bool reset_vehicle_requested() override;
    virtual void reset_vehicle(
        const OffsetAndTaitBryanAngles<float, ScenePos, 3>& location) override;
    virtual std::vector<DanglingPtr<SceneNode>> moving_nodes() const override;
    virtual void notify_race_started() override;
    virtual RaceState notify_lap_finished(
        float race_time_seconds,
        const std::string& asset_id,
        const UUVector<FixedArray<float, 3>>& vehicle_colors,
        const std::list<float>& lap_times_seconds,
        const std::list<TrackElement>& track) override;
    virtual void notify_kill(RigidBodyVehicle& rigid_body_vehicle) override;
    virtual DestructionFunctions& on_destroy_player() override;
    virtual DestructionFunctions& on_clear_vehicle() override;
    // DestructionObserver
    virtual void notify_destroyed(SceneNode& destroyed_object) override;
    virtual void notify_destroyed(const SceneVehicle& destroyed_object) override;
    virtual void notify_destroyed(const RigidBodyVehicle& destroyed_object) override;
    // IAdvanceTime
    virtual void advance_time(float dt, const StaticWorld& world) override;
    // IExternalForceProvider
    virtual void increment_external_forces(
        const std::list<RigidBodyVehicle*>& olist,
        bool burn_in,
        const PhysicsEngineConfig& cfg,
        const StaticWorld& world) override;
    
    DestructionFunctions delete_vehicle_externals;
    DestructionFunctions delete_vehicle_internals;
    VehicleMovement vehicle_movement;
    CarMovement car_movement;
    AvatarMovement avatar_movement;
private:
    void clear_opponent();
    void set_opponent(Player& opponent);
    void aim_and_shoot();
    void select_best_weapon_in_inventory();
    bool unstuck();
    const Gun& gun() const;
    Gun& gun();
    DestructionFunctions on_clear_vehicle_;
    Scene& scene_;
    CollisionQuery& collision_query_;
    VehicleSpawners& vehicle_spawners_;
    Players& players_;
    std::string id_;
    std::string team_;
    SceneVehicle* vehicle_;
    VehicleSpawner* vehicle_spawner_;
    PlayerControlled controlled_;
    std::optional<std::string> target_id_;
    DanglingPtr<SceneNode> target_scene_node_;
    RigidBodyVehicle* target_rb_;
    PlayerStats stats_;
    GameMode game_mode_;
    std::chrono::steady_clock::time_point stuck_start_;
    std::chrono::steady_clock::time_point unstuck_start_;
    UnstuckMode unstuck_mode_;
    std::string behavior_;
    float stuck_velocity_;
    float stuck_duration_;
    float unstuck_duration_;
    JoinedWayPointSandbox joined_way_point_sandbox_;
    DrivingDirection driving_direction_;
    size_t nunstucked_;
    SkillMap skills_;
    DeleteNodeMutex& delete_node_mutex_;
    VehicleSpawner* next_scene_vehicle_;
    bool reset_vehicle_to_last_checkpoint_requested_;
    std::string next_role_;
    std::map<DanglingPtr<const SceneNode>, std::string> dependent_nodes_;
    ExternalsMode externals_mode_;
    InternalsMode internals_mode_;
    SingleWaypoint single_waypoint_;
    PathfindingWaypoints pathfinding_waypoints_;
    SupplyDepotsWaypoints supply_depots_waypoints_;
    PlaybackWaypoints playback_waypoints_;
    const Focuses& focuses_;
    ScenePos select_opponent_hysteresis_factor_;
    DestructionObservers<const IPlayer&> destruction_observers_;
    const Navigate& navigate_;
    Spawner& spawner_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
};

}
