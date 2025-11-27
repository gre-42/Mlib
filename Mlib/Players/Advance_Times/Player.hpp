#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Geometry/Graph/Point_And_Flags.hpp>
#include <Mlib/Geometry/Graph/Points_And_Adjacency.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
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
#include <Mlib/Remote/Events/Times_And_Events.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <chrono>
#include <cstdint>
#include <list>
#include <mutex>
#include <optional>
#include <string>

namespace Mlib {

struct UserInfo;
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
class SupplyDepotsWaypointsCollection;
class DeleteNodeMutex;
class Bystanders;
class WeaponCycle;
class Inventory;
class CountdownPhysics;
class SceneVehicle;
class VehicleSpawner;
class VehicleSpawners;
class UserAccount;
enum class DrivingDirection: uint32_t;
enum class JoinedWayPointSandbox;
enum class WayPointLocation;
enum class WhenToEquip;

enum class GameMode: uint32_t {
    RALLY = 0x39B2A982,
    RAMMING = 0x9F250C1E,
    TEAM_DEATHMATCH = 0x4ABCD738
};

GameMode game_mode_from_string(const std::string& game_mode);
std::string game_mode_to_string(GameMode game_mode);

enum class PlayerRole: uint32_t {
    COMPETITOR = 0x7EB29427,
    BYSTANDER = 0xF362AF12
};

PlayerRole player_role_from_string(const std::string& role);
std::string player_role_to_string(PlayerRole role);

enum class ExternalsMode: uint32_t;

enum class UnstuckMode: uint32_t {
    OFF = 0x938A74FA,
    REVERSE = 0xC84AEF70,
    DELETE = 0x6EFB298A
};

UnstuckMode unstuck_mode_from_string(const std::string& unstuck_mode);
std::string unstuck_mode_to_string(UnstuckMode unstuck_mode);

enum class OpponentSelectionStrategy: uint32_t {
    KEEP = 0x2EAB9423,
    NEXT = 0x79DA3902,
    BEST = 0x1EC28956
};

enum class ControlSource;
enum class PlayerSitePrivileges;

struct PlayerControlled {
    bool has_aim_at() const;
    AimAt& aim_at();
    DanglingBaseClassPtr<SceneNode> gun_node;
};

struct SelectNextVehicleEvent {
    SelectNextVehicleQuery q;
    const std::string seat;
};

class Player final:
    public IPlayer,
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
    using ShotHistory = TimesAndEvents<std::chrono::steady_clock::time_point, std::string>;
    using SelectNextVehicleHistory = TimesAndEvents<std::chrono::steady_clock::time_point, SelectNextVehicleEvent>;

    Player(
        Scene& scene,
        SupplyDepots& supply_depots,
        const Navigate& navigate,
        const SupplyDepotsWaypointsCollection& supply_depots_waypoints_collection,
        Spawner& spawner,
        const PhysicsEngineConfig& cfg,
        CollisionQuery& collision_query,
        VehicleSpawners& vehicle_spawners,
        Players& players,
        PlayerSitePrivileges site_privileges,
        const DanglingBaseClassPtr<const UserInfo>& user_info,
        VariableAndHash<std::string> id,
        std::string team,
        std::shared_ptr<UserAccount> user_account,
        GameMode game_mode,
        PlayerRole player_role,
        UnstuckMode unstuck_mode,
        std::string behavior,
        DrivingDirection driving_direction,
        DeleteNodeMutex& delete_node_mutex,
        const CountdownPhysics& countdown_start);
    virtual ~Player() override;
    void set_skills(ControlSource control_source, const Skills& skills);
    Skills get_skills(ControlSource control_source) const;
    void set_can_drive(ControlSource control_source, bool value);
    void set_can_aim(ControlSource control_source, bool value);
    void set_can_shoot(ControlSource control_source, bool value);
    void set_can_select_weapon(ControlSource control_source, bool value);
    void set_can_select_opponent(ControlSource control_source, bool value);
    void set_select_opponent_hysteresis_factor(ScenePos factor);
    void reset_node();
    void set_scene_vehicle(SceneVehicle& vehicle, const std::string& desired_seat);
    void set_vehicle_spawner(VehicleSpawner& spawner, const std::string& desired_seat);
    DanglingBaseClassRef<RigidBodyVehicle> rigid_body();
    DanglingBaseClassRef<const RigidBodyVehicle> rigid_body() const;
    DanglingBaseClassRef<SceneNode> scene_node();
    DanglingBaseClassRef<const SceneNode> scene_node() const;
    DanglingBaseClassPtr<VehicleSpawner> next_scene_vehicle();
    const std::string& seat() const;
    const std::string& next_seat() const;
    const VariableAndHash<std::string>& scene_node_name() const;
    DanglingBaseClassRef<SceneVehicle> vehicle();
    DanglingBaseClassRef<const SceneVehicle> vehicle() const;
    DanglingBaseClassRef<VehicleSpawner> vehicle_spawner();
    DanglingBaseClassRef<const VehicleSpawner> vehicle_spawner() const;
    void set_gun_node(const DanglingBaseClassRef<SceneNode>& gun_node);
    void change_gun_node(const DanglingBaseClassPtr<SceneNode>& gun_node);
    bool has_way_points() const;
    void set_way_point_location_filter(JoinedWayPointSandbox filter);
    DanglingBaseClassPtr<const UserInfo> user_info() const;
    const std::string& team_name() const;
    DanglingBaseClassRef<Team> team();
    std::shared_ptr<UserAccount> user_account();
    PlayerStats& stats();
    const PlayerStats& stats() const;
    float car_health() const;
    GameMode game_mode() const;
    PlayerRole player_role() const;
    FixedArray<SceneDir, 3> vehicle_velocity() const;
    bool can_see(
        const RigidBodyVehicle& rbi,
        bool only_terrain = false,
        float time_offset = 0) const;
    bool can_see(
        const FixedArray<ScenePos, 3>& position,
        const FixedArray<SceneDir, 3>& velocity,
        bool only_terrain = false,
        ScenePos height_offset = 0,
        float time_offset = 0) const;
    bool can_see(
        const SceneVehicle& scene_vehicle,
        bool only_terrain = false,
        float time_offset = 0) const;
    bool can_see(
        const Player& player,
        bool only_terrain = false,
        float time_offset = 0) const;
    bool is_pedestrian() const;
    bool is_parking() const;
    bool has_scene_vehicle() const;
    bool has_vehicle_controller() const;
    std::string vehicle_name() const;
    FixedArray<float, 3> gun_direction() const;
    FixedArray<float, 3> punch_angle() const;
    bool has_weapon() const;
    bool has_gun_node() const;
    const Gun& gun() const;
    Gun& gun();
    void trigger_gun();
    bool has_weapon_cycle() const;
    Inventory& inventory();
    const Inventory& inventory() const;
    WeaponCycle& weapon_cycle();
    const WeaponCycle& weapon_cycle() const;
    void set_desired_weapon(const std::string& name, WhenToEquip when_to_equip);
    bool needs_supplies() const;
    size_t nbullets_available() const;
    std::optional<std::string> best_weapon_in_inventory() const;
    void select_opponent(OpponentSelectionStrategy strategy);
    void request_reset_vehicle_to_last_checkpoint();
    void append_dependent_node(VariableAndHash<std::string> node_name);
    void create_vehicle_externals(ExternalsMode externals_mode);
    void create_vehicle_internals(const InternalsMode& internals_mode);
    void create_gun_externals();
    void set_seat(const std::string& ui);
    void change_seat();
    const Skills& skills(ControlSource control_source) const;
    Players& players();
    bool ramming() const;
    DanglingBaseClassPtr<SceneNode> target_scene_node() const;
    DanglingBaseClassPtr<const RigidBodyVehicle> target_rb() const;
    void set_behavior(
        float stuck_velocity,
        float stuck_duration,
        float unstuck_duration,
        JoinedWayPointSandbox joined_way_point_sandbox);
    DrivingDirection driving_direction() const;
    ExternalsMode externals_mode() const;
    const InternalsMode& internals_mode() const;
    UnstuckMode unstuck_mode() const;
    const std::string& behavior() const;
    SingleWaypoint& single_waypoint();
    PathfindingWaypoints& pathfinding_waypoints();
    PlaybackWaypoints& playback_waypoints();
    PlayerSitePrivileges site_privileges() const;

    // IPlayer
    virtual const VariableAndHash<std::string>& id() const override;
    virtual std::string title() const override;
    virtual std::optional<VariableAndHash<std::string>> target_id() const override;
    virtual bool reset_vehicle_requested() override;
    virtual bool can_reset_vehicle(
        const TransformationMatrix<SceneDir, ScenePos, 3>& trafo) const override;
    virtual bool try_reset_vehicle(
        const TransformationMatrix<SceneDir, ScenePos, 3>& trafo) override;
    virtual void select_next_vehicle(
        std::chrono::steady_clock::time_point time,
        SelectNextVehicleQuery q,
        const std::string& seat) override;
    virtual void set_next_vehicle(
        VehicleSpawner& spawner,
        SceneVehicle& vehicle,
        const std::string& seat) override;
    virtual void clear_next_vehicle() override;
    virtual std::vector<DanglingBaseClassPtr<SceneNode>> moving_nodes() const override;
    virtual void notify_race_started() override;
    virtual RaceState notify_lap_finished(
        float race_time_seconds,
        const std::string& asset_id,
        const UUVector<FixedArray<float, 3>>& vehicle_colors,
        const std::list<float>& lap_times_seconds,
        const std::list<TrackElement>& track) override;
    virtual void notify_kill(RigidBodyVehicle& rigid_body_vehicle) override;
    virtual void notify_bullet_generated(std::chrono::steady_clock::time_point time) override;
    virtual DestructionFunctions& on_destroy_player() override;
    virtual DestructionFunctions& on_clear_vehicle() override;
    // IAdvanceTime
    virtual void advance_time(float dt, const StaticWorld& world) override;
    // IExternalForceProvider
    virtual void increment_external_forces(
        const PhysicsEngineConfig& cfg,
        const PhysicsPhase& phase,
        const StaticWorld& world) override;
    
    DestructionFunctions delete_vehicle_externals;
    DestructionFunctions delete_vehicle_internals;
    VehicleMovement vehicle_movement;
    CarMovement car_movement;
    AvatarMovement avatar_movement;
    ShotHistory shot_history;
    SelectNextVehicleHistory select_next_vehicle_history;
private:
    void clear_opponent();
    void set_opponent(Player& opponent);
    void aim_and_shoot();
    void select_best_weapon_in_inventory();
    bool unstuck();
    DestructionFunctions on_clear_vehicle_;
    DestructionFunctionsRemovalTokens on_clear_user_;
    DestructionFunctionsRemovalTokens on_avatar_destroyed_;
    DestructionFunctionsRemovalTokens on_vehicle_destroyed_;
    DestructionFunctionsRemovalTokens on_next_vehicle_destroyed_;
    DestructionFunctionsRemovalTokens on_target_scene_node_cleared_;
    DestructionFunctionsRemovalTokens on_target_rigid_body_destroyed_;
    DestructionFunctionsRemovalTokens on_gun_node_destroyed_;
    Scene& scene_;
    CollisionQuery& collision_query_;
    VehicleSpawners& vehicle_spawners_;
    Players& players_;
    PlayerSitePrivileges site_privileges_;
    DanglingBaseClassPtr<const UserInfo> user_info_;
    VariableAndHash<std::string> id_;
    std::string team_;
    DanglingBaseClassPtr<SceneVehicle> vehicle_;
    DanglingBaseClassPtr<VehicleSpawner> vehicle_spawner_;
    PlayerControlled controlled_;
    std::optional<VariableAndHash<std::string>> target_id_;
    DanglingBaseClassPtr<SceneNode> target_scene_node_;
    DanglingBaseClassPtr<RigidBodyVehicle> target_rb_;
    PlayerStats stats_;
    GameMode game_mode_;
    PlayerRole player_role_;
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
    DanglingBaseClassPtr<VehicleSpawner> next_scene_vehicle_;
    bool reset_vehicle_to_last_checkpoint_requested_;
    std::string next_seat_;
    std::map<
        VariableAndHash<std::string>,
        DestructionFunctionsRemovalTokens> dependent_nodes_;
    ExternalsMode externals_mode_;
    InternalsMode internals_mode_;
    SingleWaypoint single_waypoint_;
    PathfindingWaypoints pathfinding_waypoints_;
    PlaybackWaypoints playback_waypoints_;
    const CountdownPhysics& countdown_start_;
    ScenePos select_opponent_hysteresis_factor_;
    DestructionObservers<const IPlayer&> destruction_observers_;
    const Navigate& navigate_;
    const SupplyDepotsWaypointsCollection& supply_depots_waypoints_collection_;
    const SupplyDepotsWaypoints* supply_depots_waypoints_;
    Spawner& spawner_;
    std::shared_ptr<UserAccount> user_account_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
    std::chrono::steady_clock::time_point old_world_time_;
};

}
