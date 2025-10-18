#include "Player.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Components/Aim_At.hpp>
#include <Mlib/Components/Gun.hpp>
#include <Mlib/Components/Weapon_Cycle.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Ref.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Physics/Advance_Times/Bullet.hpp>
#include <Mlib/Physics/Advance_Times/Countdown_Physics.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Aim_At.hpp>
#include <Mlib/Physics/Ai/Control_Source.hpp>
#include <Mlib/Physics/Containers/Collision_Group.hpp>
#include <Mlib/Physics/Containers/Collision_Query.hpp>
#include <Mlib/Physics/Containers/Race_Identifier.hpp>
#include <Mlib/Physics/Interfaces/IDamageable.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine_Config.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Phase.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Game_Logic/Navigate.hpp>
#include <Mlib/Players/Game_Logic/Spawner.hpp>
#include <Mlib/Players/Player/Supply_Depots_Waypoints_Collection.hpp>
#include <Mlib/Players/Scene_Vehicle/Externals_Mode.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Players/Team/Team.hpp>
#include <Mlib/Players/User_Account/User_Account.hpp>
#include <Mlib/Scene_Graph/Animation/Animation_State_Updater.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Interfaces/Way_Points.hpp>
#include <Mlib/Scene_Graph/Joined_Way_Point_Sandbox.hpp>
#include <Mlib/Scene_Graph/Spawn_Arguments.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <fstream>

using namespace Mlib;

GameMode Mlib::game_mode_from_string(const std::string& game_mode) {
    static const std::map<std::string, GameMode> m{
        {"rally", GameMode::RALLY},
        {"ramming", GameMode::RAMMING},
        {"team_deathmatch", GameMode::TEAM_DEATHMATCH},
    };
    auto it = m.find(game_mode);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown game mode: \"" + game_mode + '"');
    }
    return it->second;
}

PlayerRole Mlib::player_role_from_string(const std::string& role) {
    static const std::map<std::string, PlayerRole> m{
        {"competitor", PlayerRole::COMPETITOR},
        {"bystander", PlayerRole::BYSTANDER},
    };
    auto it = m.find(role);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown player role: \"" + role + '"');
    }
    return it->second;
}

UnstuckMode Mlib::unstuck_mode_from_string(const std::string& unstuck_mode) {
    static const std::map<std::string, UnstuckMode> m{
        {"off", UnstuckMode::OFF},
        {"reverse", UnstuckMode::REVERSE},
        {"delete", UnstuckMode::DELETE}
    };
    auto it = m.find(unstuck_mode);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown unstuck mode: \"" + unstuck_mode + '"');
    }
    return it->second;
}

bool PlayerControlled::has_aim_at() const {
    return (gun_node != nullptr) && Mlib::has_aim_at(*gun_node);
}

AimAt& PlayerControlled::aim_at() {
    if (gun_node == nullptr) {
        THROW_OR_ABORT("Gun node is null");
    }
    return get_aim_at(*gun_node);
}

Player::Player(
    Scene& scene,
    SupplyDepots& supply_depots,
    const Navigate& navigate,
    const SupplyDepotsWaypointsCollection& supply_depots_waypoints_collection,
    Spawner& spawner,
    const PhysicsEngineConfig& cfg,
    CollisionQuery& collision_query,
    VehicleSpawners& vehicle_spawners,
    Players& players,
    uint32_t user_id,
    std::string user_name,
    std::string id,
    std::string team,
    std::shared_ptr<UserAccount> user_account,
    GameMode game_mode,
    PlayerRole player_role,
    UnstuckMode unstuck_mode,
    std::string behavior,
    DrivingDirection driving_direction,
    DeleteNodeMutex& delete_node_mutex,
    const CountdownPhysics& countdown_start)
    : car_movement{ *this }
    , avatar_movement{ *this }
    , on_avatar_destroyed_{ nullptr, CURRENT_SOURCE_LOCATION }
    , on_vehicle_destroyed_{ nullptr, CURRENT_SOURCE_LOCATION }
    , on_next_vehicle_destroyed_{ nullptr, CURRENT_SOURCE_LOCATION }
    , on_target_scene_node_cleared_{ nullptr, CURRENT_SOURCE_LOCATION }
    , on_target_rigid_body_destroyed_{ nullptr, CURRENT_SOURCE_LOCATION }
    , on_gun_node_destroyed_{ nullptr, CURRENT_SOURCE_LOCATION }
    , scene_{ scene }
    , collision_query_{ collision_query }
    , vehicle_spawners_{ vehicle_spawners }
    , players_{ players }
    , user_id_{ user_id }
    , user_name_{ std::move(user_name) }
    , id_{ std::move(id) }
    , team_{ std::move(team) }
    , vehicle_{ nullptr }
    , vehicle_spawner_{ nullptr }
    , controlled_{ .gun_node = nullptr }
    , target_scene_node_{ nullptr }
    , target_rb_{ nullptr }
    , game_mode_{ game_mode }
    , player_role_{ player_role }
    , unstuck_mode_{ unstuck_mode }
    , behavior_{ std::move(behavior) }
    , stuck_velocity_{ NAN }
    , stuck_duration_{ NAN }
    , unstuck_duration_{ NAN }
    , joined_way_point_sandbox_{ JoinedWayPointSandbox::NONE }
    , driving_direction_{ driving_direction }
    , nunstucked_{ 0 }
    , delete_node_mutex_{ delete_node_mutex }
    , next_scene_vehicle_{ nullptr }
    , reset_vehicle_to_last_checkpoint_requested_{ false }
    , externals_mode_{ ExternalsMode::NONE }
    , single_waypoint_{ { *this, CURRENT_SOURCE_LOCATION } }
    , pathfinding_waypoints_{ *this, cfg }
    , playback_waypoints_{ *this }
    , countdown_start_{ countdown_start }
    , select_opponent_hysteresis_factor_{ (ScenePos)0.9 }
    , destruction_observers_{ *this }
    , navigate_{ navigate }
    , supply_depots_waypoints_collection_{ supply_depots_waypoints_collection }
    , supply_depots_waypoints_{ nullptr }
    , spawner_{ spawner }
    , user_account_{ std::move(user_account) }
{
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
}

Player::~Player() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    on_destroy.clear();
    destruction_observers_.clear();
}

void Player::set_can_drive(ControlSource control_source, bool value) {
    std::scoped_lock lock{ mutex_ };
    skills_.skills(control_source).can_drive = value;
}

void Player::set_can_aim(ControlSource control_source, bool value) {
    std::scoped_lock lock{ mutex_ };
    skills_.skills(control_source).can_aim = value;
}

void Player::set_can_shoot(ControlSource control_source, bool value) {
    std::scoped_lock lock{ mutex_ };
    skills_.skills(control_source).can_shoot = value;
}

void Player::set_can_select_weapon(ControlSource control_source, bool value) {
    std::scoped_lock lock{ mutex_ };
    skills_.skills(control_source).can_select_weapon = value;
}

void Player::set_can_select_opponent(ControlSource control_source, bool value) {
    std::scoped_lock lock{ mutex_ };
    skills_.skills(control_source).can_select_opponent = value;
}

void Player::set_select_opponent_hysteresis_factor(ScenePos factor) {
    std::scoped_lock lock{ mutex_ };
    select_opponent_hysteresis_factor_ = factor;
}

void Player::reset_node() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    {
        std::scoped_lock lock{ mutex_ };
        if (vehicle_spawner_ != nullptr) {
            on_clear_vehicle_.clear();
            // The avatar can be destroyed during its dying
            // animation or while sitting in a vehicle,
            // so do not clear "on_avatar_destroyed_".
            // on_avatar_destroyed_.clear();
            on_vehicle_destroyed_.clear();
            vehicle_ = nullptr;
            vehicle_spawner_ = nullptr;
        }
        if (next_scene_vehicle_ != nullptr) {
            on_next_vehicle_destroyed_.clear();
            next_scene_vehicle_ = nullptr;
        }
        if (target_scene_node_ != nullptr) {
            clear_opponent();
        }
        if (controlled_.gun_node != nullptr) {
            if (controlled_.has_aim_at()) {
                controlled_.aim_at().set_followed(nullptr);
            }
            change_gun_node(nullptr);
        }
        vehicle_movement.reset_node();
        car_movement.reset_node();
        stuck_start_ = std::chrono::steady_clock::time_point();
        unstuck_start_ = std::chrono::steady_clock::time_point();
    }
    if (!delete_vehicle_externals.empty()) {
        delete_vehicle_externals.clear();
    }
    if (!delete_vehicle_internals.empty()) {
        delete_vehicle_internals.clear();
    }
    while (!dependent_nodes_.empty()) {
        auto name = dependent_nodes_.begin()->first;
        scene_.delete_node(name);
    }
    {
        std::scoped_lock lock{ mutex_ };
        externals_mode_ = ExternalsMode::NONE;
        internals_mode_.seat.clear();
    }
    if (vehicle_spawner_ != nullptr) {
        vehicle_spawner_->check_consistency();
    }
}

void Player::set_vehicle_spawner(
    VehicleSpawner& spawner,
    const std::string& desired_seat)
{
    std::scoped_lock lock{ mutex_ };
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (vehicle_spawner_ != nullptr) {
        THROW_OR_ABORT("Vehicle spawner already set");
    }
    if (!internals_mode_.seat.empty()) {
        THROW_OR_ABORT("Old seat is not empty");
    }
    if (desired_seat.empty()) {
        THROW_OR_ABORT("Desired seat is empty");
    }
    auto pv = spawner.get_primary_scene_vehicle();
    if (!pv->rb()->drivers_.seat_exists(desired_seat)) {
        THROW_OR_ABORT("Seat \"" + desired_seat + "\" does not exist in vehicle \"" + pv->rb()->name() + '"');
    }
    if (!pv->rb()->drivers_.seat_is_free(desired_seat)) {
        THROW_OR_ABORT("Seat \"" + desired_seat + "\" is already occupied in vehicle \"" + pv->rb()->name() + '"');
    }
    pv->rb()->drivers_.add(desired_seat, { *this, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
    if (pv->rb()->is_avatar()) {
        // The avatar can be destroyed during its dying
        // animation or while sitting in a vehicle.
        on_avatar_destroyed_.set(pv->on_destroy, CURRENT_SOURCE_LOCATION);
        on_avatar_destroyed_.add([this](){ reset_node(); }, CURRENT_SOURCE_LOCATION);
        on_vehicle_destroyed_.clear();
    } else {
        on_vehicle_destroyed_.set(pv->on_destroy, CURRENT_SOURCE_LOCATION);
        on_vehicle_destroyed_.add([this](){ reset_node(); }, CURRENT_SOURCE_LOCATION);
    }
    internals_mode_.seat = desired_seat;
    vehicle_ = pv.ptr().set_loc(CURRENT_SOURCE_LOCATION);
    vehicle_spawner_ = { &spawner, CURRENT_SOURCE_LOCATION };
}

DanglingBaseClassRef<RigidBodyVehicle> Player::rigid_body() {
    std::shared_lock lock{ mutex_ };
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!has_scene_vehicle()) {
        THROW_OR_ABORT("Player has no rigid body");
    }
    return vehicle_->rb();
}

DanglingBaseClassRef<const RigidBodyVehicle> Player::rigid_body() const {
    return const_cast<Player*>(this)->rigid_body();
}

const VariableAndHash<std::string>& Player::scene_node_name() const {
    std::shared_lock lock{ mutex_ };
    return vehicle()->scene_node_name();
}

void Player::set_gun_node(DanglingBaseClassRef<SceneNode> gun_node) {
    std::scoped_lock lock{ mutex_ };
    if (controlled_.gun_node != nullptr) {
        THROW_OR_ABORT("gun already set");
    }
    change_gun_node(gun_node.ptr());
}

void Player::change_gun_node(DanglingBaseClassPtr<SceneNode> gun_node) {
    std::scoped_lock lock{ mutex_ };
    if (controlled_.gun_node != nullptr) {
        on_gun_node_destroyed_.clear();
    }
    controlled_.gun_node = gun_node;
    if (gun_node != nullptr) {
        on_gun_node_destroyed_.set(gun_node->on_destroy, CURRENT_SOURCE_LOCATION);
        on_gun_node_destroyed_.add([this](){ controlled_.gun_node = nullptr; }, CURRENT_SOURCE_LOCATION);
    }
}

uint32_t Player::user_id() const {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    return user_id_;
}

const std::string& Player::user_name() const {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    return user_name_;
}

const std::string& Player::id() const {
    return id_;
}

std::string Player::title() const {
    if (user_account_ != nullptr) {
        return user_account_->name();
    } else {
        return id();
    }
}

const std::string& Player::team_name() const {
    std::shared_lock lock{ mutex_ };
    return team_;
}

DanglingBaseClassRef<Team> Player::team() {
    std::shared_lock lock{ mutex_ };
    return players_.get_team(team_name());
}

PlayerStats& Player::stats() {
    std::shared_lock lock{ mutex_ };
    return stats_;
}

const PlayerStats& Player::stats() const {
    std::shared_lock lock{ mutex_ };
    return stats_;
}

float Player::car_health() const {
    std::shared_lock lock{ mutex_ };
    if (has_scene_vehicle() && (vehicle_->rb()->damageable_ != nullptr)) {
        return vehicle_->rb()->damageable_->health();
    } else {
        return NAN;
    }
}

std::string Player::vehicle_name() const {
    std::shared_lock lock{ mutex_ };
    if (!has_scene_vehicle()) {
        THROW_OR_ABORT("Player has no scene vehicle, cannot get vehicle name");
    }
    return vehicle_->rb()->name();
}

GameMode Player::game_mode() const {
    std::shared_lock lock{ mutex_ };
    return game_mode_;
}

PlayerRole Player::player_role() const {
    std::shared_lock lock{ mutex_ };
    return player_role_;
}

FixedArray<SceneDir, 3> Player::vehicle_velocity() const {
    return rigid_body()->rbp_.velocity();
}

bool Player::can_see(
    const RigidBodyVehicle& rb,
    bool only_terrain,
    float time_offset) const
{
    std::shared_lock lock{ mutex_ };
    if (!has_scene_vehicle()) {
        THROW_OR_ABORT("Player::can_see requires rb");
    }
    return collision_query_.can_see(
        vehicle_->rb().get(),
        rb,
        only_terrain,
        PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK,
        time_offset);
}

bool Player::can_see(
    const FixedArray<ScenePos, 3>& position,
    const FixedArray<SceneDir, 3>& velocity,
    bool only_terrain,
    ScenePos height_offset,
    float time_offset) const
{
    std::shared_lock lock{ mutex_ };
    if (!has_scene_vehicle()) {
        THROW_OR_ABORT("Player::can_see requires rb");
    }
    return collision_query_.can_see(
        vehicle_->rb().get(),
        position,
        velocity,
        only_terrain,
        PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK,
        height_offset,
        time_offset);
}

bool Player::can_see(
    const SceneVehicle& scene_vehicle,
    bool only_terrain,
    float time_offset) const
{
    std::shared_lock lock{ mutex_ };
    if (!has_scene_vehicle()) {
        THROW_OR_ABORT("Player::can_see requires vehicle");
    }
    return collision_query_.can_see(
        vehicle_->rb().get(),
        scene_vehicle.rb().get(),
        only_terrain,
        PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK,
        time_offset);
}

bool Player::can_see(
    const Player& player,
    bool only_terrain,
    float time_offset) const
{
    std::shared_lock lock{ mutex_ };
    if (!player.has_scene_vehicle()) {
        THROW_OR_ABORT("Player::can_see requires target rb");
    }
    return can_see(
        player.vehicle().get(),
        only_terrain,
        time_offset);
}

void Player::advance_time(float dt, const StaticWorld& world) {
    std::scoped_lock lock{ mutex_ };
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    aim_and_shoot();
    select_best_weapon_in_inventory();
}

void Player::increment_external_forces(
    const PhysicsEngineConfig& cfg,
    const PhysicsPhase& phase,
    const StaticWorld& world)
{
    std::scoped_lock lock{ mutex_ };
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (phase.burn_in) {
        return;
    }
    if (!has_scene_vehicle()) {
        return;
    }
    if (!has_vehicle_controller()) {
        return;
    }
    if (!phase.group.rigid_bodies.contains(&rigid_body()->rbp_)) {
        return;
    }
    if (countdown_start_.counting()) {
        car_movement.step_on_brakes();
        car_movement.steer(0.f);
        vehicle_->rb()->vehicle_controller().apply();
        return;
    }
    bool unstucking = false;
    if ((game_mode_ == GameMode::RALLY) && (player_role_ == PlayerRole::COMPETITOR)) {
        if (playback_waypoints_.has_waypoints()) {
            playback_waypoints_.select_next_waypoint();
        }
    } else if ((unstuck_mode_ == UnstuckMode::OFF) || !(unstucking = unstuck())) {
        if (ramming()) {
            auto tpos = target_rb_->abs_target();
            single_waypoint_.set_waypoint({ tpos.casted<CompressedScenePos>(), WayPointLocation::UNKNOWN });
        } else {
            if ((supply_depots_waypoints_ == nullptr) ||
                !supply_depots_waypoints_->select_next_waypoint(*this, single_waypoint_))
            {
                pathfinding_waypoints_.select_next_waypoint();
            }
        }
    }
    if (!unstucking) {
        single_waypoint_.move_to_waypoint(skills_, world, cfg.dt_substeps(phase));
    }
}

bool Player::unstuck() {
    std::scoped_lock lock{ mutex_ };
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!has_scene_vehicle()) {
        return false;
    }
    if ((sum(squared(vehicle_->rb()->rbp_.v_com_)) > squared(stuck_velocity_)) ||
        (unstuck_start_ != std::chrono::steady_clock::time_point()))
    {
        stuck_start_ = std::chrono::steady_clock::now();
    } else if (
        (stuck_start_ != std::chrono::steady_clock::time_point()) &&
        (unstuck_start_ == std::chrono::steady_clock::time_point()) &&
        std::chrono::duration<float>(std::chrono::steady_clock::now() - stuck_start_).count() * seconds > stuck_duration_)
    {
        unstuck_start_ = std::chrono::steady_clock::now();
    }
    if (unstuck_start_ != std::chrono::steady_clock::time_point()) {
        if (std::chrono::duration<float>(std::chrono::steady_clock::now() - unstuck_start_).count() * seconds > unstuck_duration_)
        {
            unstuck_start_ = std::chrono::steady_clock::time_point();
        } else {
            // if (!waypoint_history_.empty()) {
            //     draw_waypoint_history("/tmp/" + name() + "_" + std::to_string(nunstucked_++) + ".svg");
            // }
            if (unstuck_mode_ == UnstuckMode::REVERSE) {
                car_movement.drive_backwards();
                vehicle_->rb()->vehicle_controller().steer(0, 1.f);
                vehicle_->rb()->vehicle_controller().apply();
            } else if (unstuck_mode_ == UnstuckMode::DELETE) {
                // std::scoped_lock lock{ mutex_ };
                // scene_.delete_root_node(vehicle_.scene_node_name);
                scene_.delete_root_node(vehicle_->scene_node_name());
            } else {
                THROW_OR_ABORT("Unsupported unstuck mode");
            }
            return true;
        }
    }
    return false;
}

FixedArray<float, 3> Player::gun_direction() const {
    std::shared_lock lock{ mutex_ };
    if (controlled_.gun_node == nullptr) {
        THROW_OR_ABORT("gun_direction despite gun nullptr in player \"" + id() + '"');
    }
    return -z3_from_3x3(gun().absolute_model_matrix().R);
}

FixedArray<float, 3> Player::punch_angle() const {
    std::scoped_lock lock{ mutex_ };
    if (controlled_.gun_node == nullptr) {
        THROW_OR_ABORT("punch_angle despite gun nullptr in player \"" + id() + '"');
    }
    return gun().punch_angle();
}

void Player::trigger_gun() {
    std::scoped_lock lock{ mutex_ };
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (controlled_.gun_node == nullptr) {
        THROW_OR_ABORT("Player::trigger despite gun nullptr");
    }
    gun().trigger(this, &team().get());
}

bool Player::has_gun_node() const {
    std::shared_lock lock{ mutex_ };
    return (controlled_.gun_node != nullptr);
}

Inventory& Player::inventory() {
    return rigid_body()->inventory_;
}

const Inventory& Player::inventory() const {
    return const_cast<Player*>(this)->inventory();
}

bool Player::has_weapon_cycle() const {
    return Mlib::has_weapon_cycle(scene_node());
}

WeaponCycle& Player::weapon_cycle() {
    return Mlib::get_weapon_cycle(scene_node());
}

const WeaponCycle& Player::weapon_cycle() const {
    return Mlib::get_weapon_cycle(scene_node());
}

bool Player::needs_supplies() const {
    std::shared_lock lock{ mutex_ };
    if (!has_scene_vehicle()) {
        return false;
    }
    return nbullets_available() == 0;
}

size_t Player::nbullets_available() const {
    return gun().nbullets_available();
}

std::optional<std::string> Player::best_weapon_in_inventory() const {
    std::shared_lock lock{ mutex_ };
    auto& wc = weapon_cycle();
    if ((target_rb_ == nullptr) ||
        !has_scene_vehicle())
    {
        return std::nullopt;
    }
    const auto& inventy = inventory();
    ScenePos distance_to_target = std::sqrt(sum(squared(
        target_rb_->rbp_.abs_position() - rigid_body()->rbp_.abs_position())));
    float best_score = -INFINITY;
    std::optional<std::string> best_weapon_name;
    for (const auto& [name, info] : wc.weapon_infos()) {
        if (inventy.navailable(info.ammo_type) == 0) {
            continue;
        }
        float score = info.score(distance_to_target);
        if (score > best_score) {
            best_score = score;
            best_weapon_name = name;
        }
    }
    return best_weapon_name;
}

bool Player::has_scene_vehicle() const {
    std::shared_lock lock{ mutex_ };
    if (vehicle_ == nullptr) {
        return false;
    }
    // Only checked for deleter-threads, because non-deleter-threads
    // can prevent deletion by acquiring a lock.
    if (delete_node_mutex_.this_thread_is_deleter_thread()) {
        if (vehicle_->scene_node()->shutting_down()) {
            verbose_abort("Player::has_rigid_body: Scene node shutting down");
        }
    }
    return true;
}

bool Player::has_vehicle_controller() const {
    std::shared_lock lock{ mutex_ };
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    return rigid_body()->has_vehicle_controller();
}

const Gun& Player::gun() const {
    return const_cast<Player*>(this)->gun();
}

Gun& Player::gun() {
    std::shared_lock lock{ mutex_ };
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (controlled_.gun_node == nullptr) {
        THROW_OR_ABORT("Gun node not set");
    }
    return get_gun(*controlled_.gun_node);
}

bool Player::is_pedestrian() const {
    std::shared_lock lock{ mutex_ };
    return joined_way_point_sandbox_ == JoinedWayPointSandbox::SIDEWALK;
}

bool Player::is_parking() const {
    std::shared_lock lock{ mutex_ };
    return behavior_ == "none";
}

void Player::aim_and_shoot() {
    std::scoped_lock lock{ mutex_ };
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!skills_.skills(ControlSource::AI).can_aim) {
        return;
    }
    assert_true((target_scene_node_ == nullptr) == (target_rb_ == nullptr));
    assert_true((target_scene_node_ != nullptr) == (target_id_.has_value()));
    if (skills_.skills(ControlSource::AI).can_select_opponent) {
        select_opponent(OpponentSelectionStrategy::BEST);
    } else {
        select_opponent(OpponentSelectionStrategy::KEEP);
    }
    if (!controlled_.has_aim_at()) {
        return;
    }
    assert_true((vehicle_ == nullptr) ||
                (vehicle_->scene_node().ptr() != target_scene_node_));
    controlled_.aim_at().set_followed(target_scene_node_);
    if (controlled_.gun_node == nullptr) {
        return;
    }
    if (!skills_.skills(ControlSource::AI).can_shoot) {
        return;
    }
    if ((target_scene_node_ != nullptr) && (controlled_.aim_at().target_locked_on())) {
        gun().trigger(this, &team().get());
    }
}

void Player::select_best_weapon_in_inventory() {
    std::scoped_lock lock{ mutex_ };
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!skills_.skills(ControlSource::AI).can_select_weapon) {
        return;
    }
    if (!has_scene_vehicle()) {
        return;
    }
    if (!has_weapon_cycle()) {
        return;
    }
    auto best_weapon = best_weapon_in_inventory();
    if (!best_weapon.has_value()) {
        return;
    }
    weapon_cycle().set_desired_weapon(id_, *best_weapon);
}

bool Player::ramming() const {
    std::shared_lock lock{ mutex_ };
    return (game_mode_ == GameMode::RAMMING) && (target_rb_ != nullptr);
}

std::optional<VariableAndHash<std::string>> Player::target_id() const {
    std::shared_lock lock{ mutex_ };
    return target_id_;
}

DanglingBaseClassPtr<SceneNode> Player::target_scene_node() const {
    std::shared_lock lock{ mutex_ };
    return target_scene_node_;
}

DanglingBaseClassPtr<const RigidBodyVehicle> Player::target_rb() const {
    std::shared_lock lock{ mutex_ };
    return target_rb_;
}

void Player::select_opponent(OpponentSelectionStrategy strategy) {
    std::scoped_lock lock{ mutex_ };
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!has_scene_vehicle()) {
        return;
    }
    if (players_.players().empty()) {
        THROW_OR_ABORT("List of players is empty");
    }
    size_t current_opponent_index = SIZE_MAX;
    std::vector<Player*> players_vec;
    players_vec.reserve(players_.players().size());
    for (auto& [_, p] : players_.players()) {
        if ((target_scene_node_ != nullptr) &&
            (p->vehicle_ != nullptr) &&
            (target_scene_node_ == p->vehicle_->scene_node().ptr()))
        {
            if (current_opponent_index != SIZE_MAX) {
                THROW_OR_ABORT("Found multiple players with target node");
            }
            current_opponent_index = players_vec.size();
        }
        players_vec.push_back(&p.get());
    }
    auto opponent_score = [&](size_t i) {
        auto& p = *players_vec[i];
        if (p.team_ == team_) {
            return -(ScenePos)INFINITY;
        }
        if (!p.has_scene_vehicle()) {
            return -(ScenePos)INFINITY;
        }
        if (!can_see(p.rigid_body().get())) {
            return -(ScenePos)INFINITY;
        }
        switch (strategy) {
        case OpponentSelectionStrategy::KEEP: {
            if (i == current_opponent_index) {
                return (ScenePos)INFINITY;
            } else {
                return -(ScenePos)INFINITY;
            }
        }
        case OpponentSelectionStrategy::NEXT: {
            if (i == current_opponent_index) {
                return -(ScenePos)INFINITY;
            } else {
                return (ScenePos)INFINITY;
            }
        }
        case OpponentSelectionStrategy::BEST: {
            auto dist_squared = sum(squared(players_vec[i]->rigid_body()->rbp_.abs_position() - this->rigid_body()->rbp_.abs_position()));
            if (i == current_opponent_index) {
                dist_squared *= squared(select_opponent_hysteresis_factor_);
            }
            return -dist_squared;
        }
        }
        THROW_OR_ABORT("Unknown opponent selection strategy");
        };
    ScenePos best_score = -INFINITY;
    Player* best_opponent = nullptr;
    if (current_opponent_index != SIZE_MAX) {
        best_score = opponent_score(current_opponent_index);
        if (best_score == -INFINITY) {
            clear_opponent();
        } else {
            best_opponent = players_vec[current_opponent_index];
        }
    }
    size_t i = current_opponent_index;
    while (best_score != INFINITY) {
        if ((current_opponent_index == SIZE_MAX) &&
            (i == players_vec.size() - 1))
        {
            break;
        }
        i = (i + 1) % players_vec.size();
        if ((current_opponent_index != SIZE_MAX) &&
            (i == current_opponent_index))
        {
            break;
        }
        Player& p = *players_vec[i];
        if (p.team_ != team_) {
            if (!p.has_scene_vehicle()) {
                continue;
            }
            ScenePos candidate_score = opponent_score(i);
            if (candidate_score > best_score) {
                best_score = candidate_score;
                best_opponent = &p;
            }
        }
    }
    if (best_opponent != nullptr) {
        if (best_opponent->vehicle_->scene_node().ptr() == target_scene_node_) {
            return;
        }
        if (target_scene_node_ != nullptr) {
            clear_opponent();
        }
        set_opponent(*best_opponent);
    }
}

void Player::clear_opponent() {
    std::scoped_lock lock{ mutex_ };
    if (target_scene_node_ == nullptr) {
        THROW_OR_ABORT("Player has no opponent");
    }
    on_target_scene_node_cleared_.clear();
    on_target_rigid_body_destroyed_.clear();
    target_id_.reset();
    target_scene_node_ = nullptr;
    target_rb_ = nullptr;
}

void Player::set_opponent(Player& opponent) {
    std::scoped_lock lock{ mutex_ };
    if (target_scene_node_ != nullptr) {
        THROW_OR_ABORT("Player already has an opponent");
    }
    if (opponent.vehicle_ == nullptr) {
        THROW_OR_ABORT("Opponent has no avatar or vehicle");
    }
    target_id_ = opponent.vehicle_->scene_node_name();
    target_scene_node_ = opponent.vehicle_->scene_node().ptr();
    target_rb_ = opponent.vehicle_->rb().ptr().set_loc(CURRENT_SOURCE_LOCATION);
    on_target_scene_node_cleared_.set(target_scene_node_->on_clear, CURRENT_SOURCE_LOCATION);
    on_target_scene_node_cleared_.add([this](){ clear_opponent(); }, CURRENT_SOURCE_LOCATION);
    on_target_rigid_body_destroyed_.set(target_rb_->on_destroy, CURRENT_SOURCE_LOCATION);
    on_target_rigid_body_destroyed_.add([this](){ clear_opponent(); }, CURRENT_SOURCE_LOCATION);
}

DanglingBaseClassRef<SceneNode> Player::scene_node() {
    std::shared_lock lock{ mutex_ };
    if (!has_scene_vehicle()) {
        THROW_OR_ABORT("Player has no scene node");
    }
    return vehicle_->scene_node();
}

DanglingBaseClassRef<const SceneNode> Player::scene_node() const {
    return const_cast<Player*>(this)->scene_node();
}

DanglingBaseClassPtr<VehicleSpawner> Player::next_scene_vehicle() {
    return next_scene_vehicle_;
}

const std::string& Player::next_seat() const {
    std::shared_lock lock{ mutex_ };
    return next_seat_;
}

DanglingBaseClassRef<SceneVehicle> Player::vehicle() {
    std::shared_lock lock{ mutex_ };
    if (vehicle_ == nullptr) {
        THROW_OR_ABORT("Vehicle is null");
    }
    return *vehicle_;
}

DanglingBaseClassRef<const SceneVehicle> Player::vehicle() const {
    return const_cast<Player*>(this)->vehicle();
}

DanglingBaseClassRef<VehicleSpawner> Player::vehicle_spawner() {
    if (vehicle_spawner_ == nullptr) {
        THROW_OR_ABORT("Player has no vehicle spawner");
    }
    return *vehicle_spawner_;
}

DanglingBaseClassRef<const VehicleSpawner> Player::vehicle_spawner() const {
    return const_cast<Player*>(this)->vehicle_spawner();
}

void Player::set_next_vehicle(
    VehicleSpawner& spawner,
    SceneVehicle& vehicle,
    const std::string& seat)
{
    clear_next_vehicle();
    next_scene_vehicle_ = { &spawner, CURRENT_SOURCE_LOCATION };
    next_seat_ = seat;
    on_next_vehicle_destroyed_.set(vehicle.on_destroy, CURRENT_SOURCE_LOCATION);
    on_next_vehicle_destroyed_.add([this](){
        next_scene_vehicle_ = nullptr;
    }, CURRENT_SOURCE_LOCATION);
}

void Player::clear_next_vehicle() {
    if (next_scene_vehicle_ != nullptr) {
        on_next_vehicle_destroyed_.clear();
        next_scene_vehicle_ = nullptr;
    }
}

struct AvailableSpawner {
    VehicleSpawner& spawner;
    SceneVehicle& vehicle;
    const std::string& seat;
};

void Player::select_next_vehicle(
    SelectNextVehicleQuery q,
    const std::string& seat)
{
    std::scoped_lock lock{ mutex_ };
    if (!has_scene_vehicle()) {
        return;
    }
    clear_next_vehicle();
    std::optional<AvailableSpawner> closest_spawner;
    ScenePos closest_distance2 = INFINITY;
    for (auto& [_, s] : vehicle_spawners_.spawners()) {
        if (!s->has_scene_vehicle()) {
            continue;
        }
        if (vehicle_spawner_.get() == s.get()) {
            continue;
        }
        auto v = s->get_primary_scene_vehicle();
        if (any(q & SelectNextVehicleQuery::EXIT)) {
            if (s->has_player() && (&s->get_player().get() == this)) {
                const auto* free_seat = v->rb()->drivers_.first_free_seat();
                if (free_seat == nullptr) {
                    THROW_OR_ABORT("Avatar \"" + v->rb()->name() + "\" has no free seat");
                }
                set_next_vehicle(*s, v.get(), *free_seat);
                return;
            }
        }
        if (any(q & SelectNextVehicleQuery::ANY_ENTER) &&
            rigid_body()->is_avatar() &&
            !v->rb()->is_avatar())
        {
            ScenePos dist2 = sum(squared(v->rb()->rbp_.abs_position() - vehicle_->rb()->rbp_.abs_position()));
            if (dist2 < closest_distance2) {
                closest_spawner.reset();
                if (!seat.empty()) {
                    closest_spawner.emplace(*s, v.get(), seat);
                } else {
                    const auto* free_seat = v->rb()->drivers_.first_free_seat();
                    if (free_seat != nullptr) {
                        closest_spawner.emplace(*s, v.get(), *free_seat);
                    } else {
                        closest_spawner.emplace(*s, v.get(), "driver");
                    }
                }
                closest_distance2 = dist2;
            }
        }
    }
    if (closest_spawner.has_value()) {
        auto driver = closest_spawner->vehicle.rb()->drivers_.try_get(seat);
        if (driver != nullptr) {
            if (!any(q & SelectNextVehicleQuery::ENTER_BY_FORCE)) {
                return;
            }
            driver->select_next_vehicle(SelectNextVehicleQuery::EXIT, "driver");
        }
        set_next_vehicle(
            closest_spawner->spawner,
            closest_spawner->vehicle,
            closest_spawner->seat);
    }
}

void Player::request_reset_vehicle_to_last_checkpoint() {
    std::scoped_lock lock{ mutex_ };
    reset_vehicle_to_last_checkpoint_requested_ = true;
}

bool Player::reset_vehicle_requested() {
    std::scoped_lock lock{ mutex_ };
    bool result = reset_vehicle_to_last_checkpoint_requested_;
    reset_vehicle_to_last_checkpoint_requested_ = false;
    return result;
}

bool Player::can_reset_vehicle(
    const TransformationMatrix<SceneDir, ScenePos, 3>& trafo) const
{
    if (vehicle_spawner_ == nullptr) {
        THROW_OR_ABORT("Player has no vehicle spawner");
    }
    return spawner_.can_spawn_at_spawn_point(
        *vehicle_spawner_.get(),
        trafo.casted<SceneDir, CompressedScenePos>(),
        AxisAlignedBoundingBox<CompressedScenePos, 3>::zero());
}

bool Player::try_reset_vehicle(
    const TransformationMatrix<SceneDir, ScenePos, 3>& trafo)
{
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    {
        std::scoped_lock lock{ mutex_ };
        reset_vehicle_to_last_checkpoint_requested_ = false;    
    }
    if (vehicle_spawner_ == nullptr) {
        return false;
    }
    if (!has_scene_vehicle()) {
        return false;
    }
    auto vs = vehicle_spawner_;
    vehicle_spawner_->delete_vehicle();
    if (vehicle_spawner_ != nullptr) {
        verbose_abort("Vehicle spawner not null after deletion");
    }
    if (!spawner_.try_spawn_at_spawn_point(
        *vs.get(),
        trafo.casted<SceneDir, CompressedScenePos>(),
        AxisAlignedBoundingBox<CompressedScenePos, 3>::zero()))
    {
        if (vehicle_spawner_ != nullptr) {
            verbose_abort("Vehicle spawner not null after failed spawning");
        }
        return false;
    } else {
        if (vehicle_spawner_ == nullptr) {
            verbose_abort("Vehicle spawner is null after spawning");
        }
        return true;
    }
}

std::vector<DanglingBaseClassPtr<SceneNode>> Player::moving_nodes() const {
    std::vector<DanglingBaseClassPtr<SceneNode>> result;
    const auto vs = vehicle_spawner()->get_scene_vehicles(CURRENT_SOURCE_LOCATION);
    result.reserve(vs->size());
    for (const auto& v : vs.get()) {
        result.push_back(v->scene_node().ptr());
    }
    return result;
}

void Player::create_vehicle_externals(ExternalsMode externals_mode) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (externals_mode_ != ExternalsMode::NONE) {
        THROW_OR_ABORT("Externals already created (0)");
    }
    if (!has_scene_vehicle()) {
        THROW_OR_ABORT("Create vehicle externals without scene vehicle");
    }
    if (!delete_vehicle_externals.empty()) {
        delete_vehicle_externals.print_source_locations();
        THROW_OR_ABORT("Vehicle externals set after deleters were added");
    }
    if (!delete_vehicle_internals.empty()) {
        delete_vehicle_internals.print_source_locations();
        THROW_OR_ABORT("Vehicle internals not empty while adding externals");
    }
    vehicle_->create_vehicle_externals(*this, externals_mode);
    std::scoped_lock lock{ mutex_ };
    externals_mode_ = externals_mode;
}

void Player::create_vehicle_internals(const InternalsMode& internals_mode) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (externals_mode_ == ExternalsMode::NONE) {
        THROW_OR_ABORT("Internals set before vehicle externals");
    }
    if (internals_mode.seat.empty()) {
        THROW_OR_ABORT("Attempt to set empty seat");
    }
    if (!has_scene_vehicle()) {
        THROW_OR_ABORT("Create internals without scene vehicle");
    }
    if (!delete_vehicle_internals.empty()) {
        delete_vehicle_internals.print_source_locations();
        THROW_OR_ABORT("Create internals set after deleters were added");
    }
    vehicle_->create_vehicle_internals(*this, internals_mode);
    std::scoped_lock lock{ mutex_ };
    internals_mode_ = internals_mode;
}

void Player::create_gun_externals() {
    if (has_weapon_cycle()) {
        weapon_cycle().create_externals(id());
    }
}

void Player::set_seat(const std::string& seat) {
    std::scoped_lock lock{ mutex_ };
    if (externals_mode_ == ExternalsMode::NONE) {
        THROW_OR_ABORT("Attempt to set seat before vehicle externals");
    }
    if (!delete_vehicle_internals.empty()) {
        delete_vehicle_internals.clear();
    }
    InternalsMode new_mode{
        .seat = seat
    };
    create_vehicle_internals(new_mode);
}

void Player::change_seat() {
    std::scoped_lock lock{ mutex_ };
    auto* new_seat = rigid_body()->drivers_.next_free_seat(internals_mode_.seat);
    if (new_seat == nullptr) {
        return;
    }
    set_seat(*new_seat);
}

const Skills& Player::skills(ControlSource control_source) const {
    std::shared_lock lock{ mutex_ };
    return skills_.skills(control_source);
}

Players& Player::players() {
    std::shared_lock lock{ mutex_ };
    return players_;
}

void Player::set_behavior(
    float stuck_velocity,
    float stuck_duration,
    float unstuck_duration,
    JoinedWayPointSandbox joined_way_point_sandbox)
{
    std::scoped_lock lock{ mutex_ };
    stuck_velocity_ = stuck_velocity;
    stuck_duration_ = stuck_duration;
    unstuck_duration_ = unstuck_duration;
    joined_way_point_sandbox_ = joined_way_point_sandbox;
}

DrivingDirection Player::driving_direction() const {
    std::shared_lock lock{ mutex_ };
    return driving_direction_;
}

ExternalsMode Player::externals_mode() const {
    std::shared_lock lock{ mutex_ };
    return externals_mode_;
}

const InternalsMode& Player::internals_mode() const {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    return internals_mode_;
}

const std::string& Player::behavior() const {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    return behavior_;
}

SingleWaypoint& Player::single_waypoint() {
    return single_waypoint_;
}

PathfindingWaypoints& Player::pathfinding_waypoints() {
    if (game_mode_ == GameMode::RALLY) {
        THROW_OR_ABORT("Player::pathfinding_waypoints called, but game mode is rally");
    }
    return pathfinding_waypoints_;
}

PlaybackWaypoints& Player::playback_waypoints() {
    if (game_mode_ != GameMode::RALLY) {
        THROW_OR_ABORT("Player::playback_waypoints called, but game mode is not rally");
    }
    return playback_waypoints_;
}

void Player::append_dependent_node(VariableAndHash<std::string> node_name) {
    std::scoped_lock lock{ mutex_ };
    auto node = scene_.get_node(node_name, DP_LOC);
    auto res = dependent_nodes_.try_emplace(std::move(node_name), node->on_clear, CURRENT_SOURCE_LOCATION);
    if (!res.second) {
        THROW_OR_ABORT("Node \"" + *node_name + "\" already is a dependent node of player \"" + id() + '"');
    }
    res.first->second.add(
        [this, it=res.first->first](){ dependent_nodes_.erase(it); },
        CURRENT_SOURCE_LOCATION);
}

void Player::notify_race_started() {
    std::scoped_lock lock{ mutex_ };
    stats_ = PlayerStats();
}

RaceState Player::notify_lap_finished(
    float race_time_seconds,
    const std::string& asset_id,
    const UUVector<FixedArray<float, 3>>& vehicle_colors,
    const std::list<float>& lap_times_seconds,
    const std::list<TrackElement>& track)
{
    std::scoped_lock lock{ mutex_ };
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (lap_times_seconds.empty()) {
        THROW_OR_ABORT("Lap times list is empty");
    }
    stats_.best_lap_time = std::min(stats_.best_lap_time, lap_times_seconds.back());
    stats_.nlaps = integral_cast<uint32_t>(lap_times_seconds.size());
    if (lap_times_seconds.size() == players_.race_identifier().laps) {
        stats_.rank = players_.rank(race_time_seconds);
        stats_.race_time = race_time_seconds;
    }
    return players_.notify_lap_finished(
        this,
        asset_id,
        vehicle_colors,
        race_time_seconds,
        lap_times_seconds,
        track);
}

void Player::notify_kill(RigidBodyVehicle& rigid_body_vehicle) {
    std::scoped_lock lock{ mutex_ };
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    for (const auto& [_, iplayer] : rigid_body_vehicle.drivers_.players_map()) {
        Player* player = dynamic_cast<Player*>(&iplayer.get());
        if (player == nullptr) {
            THROW_OR_ABORT("Driver is not a player");
        }
        if (player->team_name() != team_name()) {
            ++stats_.nkills;
        }
    }
}

DestructionFunctions& Player::on_destroy_player() {
    return on_destroy;
}

DestructionFunctions& Player::on_clear_vehicle() {
    return on_clear_vehicle_;
}

bool Player::has_way_points() const {
    std::shared_lock lock{ mutex_ };
    return navigate_.has_way_points() && any(joined_way_point_sandbox_);
}

void Player::set_way_point_location_filter(JoinedWayPointSandbox filter) {
    std::scoped_lock lock{ mutex_ };
    if (!navigate_.has_way_points()) {
        THROW_OR_ABORT("Player \"" + id_ + "\" has no waypoints");
    }
    auto final_filter = joined_way_point_sandbox_ & filter;
    size_t nfound = 0;
    for (const auto& [location, wp] : navigate_.way_points()) {
        if (!any(location & final_filter)) {
            continue;
        }
        if (nfound != 0) {
            THROW_OR_ABORT(
                "Player \"" + id_ + "\": Found multiple waypoints for final filter \"" +
                joined_way_point_sandbox_to_string(final_filter) + '"');
        }
        pathfinding_waypoints_.set_waypoints(wp);
        supply_depots_waypoints_ = &supply_depots_waypoints_collection_.get_way_points(location);
        ++nfound;
    }
    if (nfound == 0) {
        THROW_OR_ABORT(
            "Player \"" + id_ + "\": Could not find waypoints for final filter \"" +
            joined_way_point_sandbox_to_string(final_filter) + '"');
    }
}
