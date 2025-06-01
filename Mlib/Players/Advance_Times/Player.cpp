#include "Player.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Components/Aim_At.hpp>
#include <Mlib/Components/Gun.hpp>
#include <Mlib/Components/Weapon_Cycle.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Object.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Physics/Advance_Times/Bullet.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Aim_At.hpp>
#include <Mlib/Physics/Ai/Control_Source.hpp>
#include <Mlib/Physics/Containers/Collision_Query.hpp>
#include <Mlib/Physics/Containers/Race_Identifier.hpp>
#include <Mlib/Physics/Interfaces/IDamageable.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Game_Logic/Navigate.hpp>
#include <Mlib/Players/Game_Logic/Spawner.hpp>
#include <Mlib/Players/Scene_Vehicle/Externals_Mode.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Players/Team/Team.hpp>
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
        {"ramming", GameMode::RAMMING},
        {"team_deathmatch", GameMode::TEAM_DEATHMATCH},
        {"rally", GameMode::RALLY},
        {"bystander", GameMode::BYSTANDER},
    };
    auto it = m.find(game_mode);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown game mode: \"" + game_mode + '"');
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
    Spawner& spawner,
    const PhysicsEngineConfig& cfg,
    CollisionQuery& collision_query,
    VehicleSpawners& vehicle_spawners,
    Players& players,
    uint32_t user_id,
    std::string user_name,
    std::string id,
    std::string team,
    GameMode game_mode,
    UnstuckMode unstuck_mode,
    std::string behavior,
    DrivingDirection driving_direction,
    DeleteNodeMutex& delete_node_mutex,
    const Focuses& focuses)
    : car_movement{ *this }
    , avatar_movement{ *this }
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
    , supply_depots_waypoints_{ *this, single_waypoint_, supply_depots }
    , playback_waypoints_{ *this }
    , focuses_{ focuses }
    , select_opponent_hysteresis_factor_{ (ScenePos)0.9 }
    , destruction_observers_{ *this }
    , navigate_{ navigate }
    , spawner_{ spawner }
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
    {
        std::scoped_lock lock{ mutex_ };
        if (vehicle_spawner_ != nullptr) {
            on_clear_vehicle_.clear();
            vehicle_->destruction_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
            vehicle_ = nullptr;
            vehicle_spawner_ = nullptr;
        }
        if (next_scene_vehicle_ != nullptr) {
            next_scene_vehicle_->get_primary_scene_vehicle()->destruction_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
            next_scene_vehicle_ = nullptr;
        }
        if (target_scene_node_ != nullptr) {
            target_scene_node_->clearing_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
            target_id_.reset();
            target_scene_node_ = nullptr;
            target_rb_ = nullptr;
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
    if (!dependent_nodes_.empty()) {
        clear_map_recursively(dependent_nodes_, [this](const auto& p){
            p.key() = nullptr;
            scene_.delete_node(p.mapped());
            });
    }
    {
        std::scoped_lock lock{ mutex_ };
        externals_mode_ = ExternalsMode::NONE;
        internals_mode_.role.clear();
    }
}

void Player::set_vehicle_spawner(
    VehicleSpawner& spawner,
    const std::string& desired_role)
{
    std::scoped_lock lock{ mutex_ };
    if (vehicle_spawner_ != nullptr) {
        THROW_OR_ABORT("Vehicle spawner already set");
    }
    if (!internals_mode_.role.empty()) {
        THROW_OR_ABORT("Old role is not empty");
    }
    if (desired_role.empty()) {
        THROW_OR_ABORT("Desired role is empty");
    }
    auto pv = spawner.get_primary_scene_vehicle();
    if (!pv->rb()->drivers_.role_exists(desired_role)) {
        THROW_OR_ABORT("Role \"" + desired_role + "\" does not exist in vehicle \"" + pv->rb()->name() + '"');
    }
    if (!pv->rb()->drivers_.role_is_free(desired_role)) {
        THROW_OR_ABORT("Role \"" + desired_role + "\" is already occupied in vehicle \"" + pv->rb()->name() + '"');
    }
    pv->rb()->drivers_.add(desired_role, { *this, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
    pv->destruction_observers.add({ *this, CURRENT_SOURCE_LOCATION });
    internals_mode_.role = desired_role;
    vehicle_ = pv.ptr();
    vehicle_spawner_ = &spawner;
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

void Player::set_gun_node(DanglingRef<SceneNode> gun_node) {
    std::scoped_lock lock{ mutex_ };
    if (controlled_.gun_node != nullptr) {
        THROW_OR_ABORT("gun already set");
    }
    change_gun_node(gun_node.ptr());
}

void Player::change_gun_node(DanglingPtr<SceneNode> gun_node) {
    std::scoped_lock lock{ mutex_ };
    if (controlled_.gun_node != nullptr) {
        controlled_.gun_node->destruction_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
    }
    controlled_.gun_node = gun_node;
    if (gun_node != nullptr) {
        gun_node->destruction_observers.add({ *this, CURRENT_SOURCE_LOCATION });
    }
}

uint32_t Player::user_id() const {
    return user_id_;
}

const std::string& Player::user_name() const {
    return user_name_;
}

std::string Player::id() const {
    std::shared_lock lock{ mutex_ };
    return id_;
}

std::string Player::title() const {
    return id();
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
    std::shared_lock lock0{ mutex_ };
    if (!has_scene_vehicle()) {
        THROW_OR_ABORT("Player has no scene vehicle, cannot get vehicle name");
    }
    return vehicle_->rb()->name();
}

GameMode Player::game_mode() const {
    std::shared_lock lock{ mutex_ };
    return game_mode_;
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
    const FixedArray<ScenePos, 3>& pos,
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
        pos,
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

void Player::notify_destroyed(SceneNode& destroyed_object) {
    std::scoped_lock lock{ mutex_ };
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (&destroyed_object == &target_scene_node_.obj()) {
        target_id_.reset();
        target_scene_node_ = nullptr;
        target_rb_ = nullptr;
    }
    if (&destroyed_object == &controlled_.gun_node.obj()) {
        controlled_.gun_node = nullptr;
    }
    // The node has already removed the player from its observers,
    // so nothing no deregistration is done here.
    dependent_nodes_.erase(DanglingRef<SceneNode>::from_object(destroyed_object, DP_LOC).ptr());
}

void Player::notify_destroyed(const SceneVehicle& destroyed_object) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if ((next_scene_vehicle_ != nullptr) &&
        (&destroyed_object == &next_scene_vehicle_->get_primary_scene_vehicle().get()))
    {
        next_scene_vehicle_ = nullptr;
    }
    if (&destroyed_object == vehicle_.get()) {
        vehicle_ = nullptr;
        vehicle_spawner_ = nullptr;
        reset_node();
    }
}

void Player::notify_destroyed(const RigidBodyVehicle& destroyed_object) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (vehicle_ == nullptr) {
        verbose_abort("Player::notify_destroyed: Vehicle is null");
    }
    if (vehicle_spawner_ == nullptr) {
        verbose_abort("Player::notify_destroyed: Vehicle spawner is null");
    }
    if (&destroyed_object != &rigid_body().get()) {
        verbose_abort("Player::notify_destroyed: Unexpected rigid body vehicle");
    }
    if (destroyed_object.drivers_.try_get(internals_mode_.role).get() != this) {
        verbose_abort("Player::notify_destroyed: Destroyed object's player with role \"" + internals_mode_.role + "\" is not this player");
    }
    reset_node();
}

void Player::advance_time(float dt, const StaticWorld& world) {
    std::scoped_lock lock{ mutex_ };
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    aim_and_shoot();
    select_best_weapon_in_inventory();
}

void Player::increment_external_forces(
    bool burn_in,
    const PhysicsEngineConfig& cfg,
    const StaticWorld& world)
{
    std::scoped_lock lock{ mutex_ };
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (burn_in) {
        return;
    }
    if (!has_scene_vehicle()) {
        return;
    }
    if (!has_vehicle_controller()) {
        return;
    }
    {
        bool countdown_active;
        {
            std::shared_lock lock1{ focuses_.mutex };
            countdown_active = focuses_.countdown_active();
        }
        if (countdown_active) {
            car_movement.step_on_brakes();
            car_movement.steer(0.f);
            vehicle_->rb()->vehicle_controller().apply();
            return;
        }
    }
    bool unstucking = false;
    if (game_mode_ == GameMode::RALLY) {
        if (playback_waypoints_.has_waypoints()) {
            playback_waypoints_.select_next_waypoint();
        }
    } else if ((unstuck_mode_ == UnstuckMode::OFF) || !(unstucking = unstuck())) {
        if (ramming()) {
            auto tpos = target_rb_->abs_target();
            single_waypoint_.set_waypoint({ tpos.casted<CompressedScenePos>(), WayPointLocation::UNKNOWN });
        } else {
            if (!supply_depots_waypoints_.select_next_waypoint()) {
                pathfinding_waypoints_.select_next_waypoint();
            }
        }
    }
    if (!unstucking) {
        single_waypoint_.move_to_waypoint(skills_, world);
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
                scene_.schedule_delete_root_node(vehicle_->scene_node_name());
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
    if (scene_node_scheduled_for_deletion()) {
        THROW_OR_ABORT("Attempt to trigger gun despite scene node scheduled for deletion");
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
        if (scene_.root_node_scheduled_for_deletion(vehicle_->scene_node_name())) {
            return false;
        }
    }
    return true;
}

bool Player::has_vehicle_controller() const {
    std::shared_lock lock{ mutex_ };
    return rigid_body()->has_vehicle_controller();
}

const Gun& Player::gun() const {
    return const_cast<Player*>(this)->gun();
}

Gun& Player::gun() {
    std::shared_lock lock{ mutex_ };
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

DanglingPtr<SceneNode> Player::target_scene_node() const {
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
        players_vec.push_back(p.get());
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
    target_scene_node_->clearing_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
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
    target_rb_ = opponent.vehicle_->rb().ptr();
    target_scene_node_->clearing_observers.add({ *this, CURRENT_SOURCE_LOCATION });
}

DanglingRef<SceneNode> Player::scene_node() {
    std::shared_lock lock{ mutex_ };
    if (!has_scene_vehicle()) {
        THROW_OR_ABORT("Player has no scene node");
    }
    return vehicle_->scene_node();
}

DanglingRef<const SceneNode> Player::scene_node() const {
    return const_cast<Player*>(this)->scene_node();
}

bool Player::scene_node_scheduled_for_deletion() const {
    return scene_.root_node_scheduled_for_deletion(vehicle()->scene_node_name());
}

VehicleSpawner* Player::next_scene_vehicle() {
    return next_scene_vehicle_;
}

const std::string& Player::next_role() const {
    std::shared_lock lock{ mutex_ };
    return next_role_;
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

VehicleSpawner& Player::vehicle_spawner() {
    if (vehicle_spawner_ == nullptr) {
        THROW_OR_ABORT("Player has no vehicle spawner");
    }
    return *vehicle_spawner_;
}

const VehicleSpawner& Player::vehicle_spawner() const {
    return const_cast<Player*>(this)->vehicle_spawner();
}

void Player::select_next_vehicle() {
    std::scoped_lock lock{ mutex_ };
    if (!has_scene_vehicle()) {
        return;
    }
    ScenePos closest_distance2 = INFINITY;
    auto clear_next_scene_vehicle = [this](){
        if (next_scene_vehicle_ != nullptr) {
            next_scene_vehicle_->get_primary_scene_vehicle()->destruction_observers.remove({ *this, CURRENT_SOURCE_LOCATION });
            next_scene_vehicle_ = nullptr;
        }
    };
    clear_next_scene_vehicle();
    for (auto& [_, s] : vehicle_spawners_.spawners()) {
        if (!s->has_scene_vehicle()) {
            continue;
        }
        if (vehicle_spawner_ == s.get()) {
            continue;
        }
        auto v = s->get_primary_scene_vehicle();
        const auto* free_role = v->rb()->drivers_.first_free_role();
        if (free_role == nullptr) {
            continue;
        }
        auto set_next_scene_vehicle = [&](){
            clear_next_scene_vehicle();
            next_scene_vehicle_ = s.get();
            next_role_ = *free_role;
            v->destruction_observers.add({ *this, CURRENT_SOURCE_LOCATION });
        };
        if (s->has_player() && (&s->get_player().get() == this)) {
            set_next_scene_vehicle();
            break;
        }
        ScenePos dist2 = sum(squared(v->rb()->rbp_.abs_position() - vehicle_->rb()->rbp_.abs_position()));
        if (dist2 < closest_distance2) {
            set_next_scene_vehicle();
            closest_distance2 = dist2;
        }
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
        *vehicle_spawner_,
        trafo.casted<SceneDir, CompressedScenePos>());
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
        *vs,
        trafo.casted<SceneDir, CompressedScenePos>()))
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

std::vector<DanglingPtr<SceneNode>> Player::moving_nodes() const {
    std::vector<DanglingPtr<SceneNode>> result;
    const auto& vs = vehicle_spawner().get_scene_vehicles();
    result.reserve(vs.size());
    for (const auto& v : vs) {
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
    vehicle_->create_vehicle_externals(user_id_, user_name_, id(), externals_mode, behavior_);
    std::scoped_lock lock{ mutex_ };
    externals_mode_ = externals_mode;
}

void Player::create_vehicle_internals(const InternalsMode& internals_mode) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (externals_mode_ == ExternalsMode::NONE) {
        THROW_OR_ABORT("Internals set before vehicle externals");
    }
    if (internals_mode.role.empty()) {
        THROW_OR_ABORT("Attempt to set empty role");
    }
    if (!has_scene_vehicle()) {
        THROW_OR_ABORT("Create internals without scene vehicle");
    }
    if (!delete_vehicle_internals.empty()) {
        delete_vehicle_internals.print_source_locations();
        THROW_OR_ABORT("Create internals set after deleters were added");
    }
    vehicle_->create_vehicle_internals(user_id_, user_name_, id(), externals_mode_, skills_, behavior_, internals_mode);
    std::scoped_lock lock{ mutex_ };
    internals_mode_ = internals_mode;
}

void Player::set_role(const std::string& role) {
    std::scoped_lock lock{ mutex_ };
    if (externals_mode_ == ExternalsMode::NONE) {
        THROW_OR_ABORT("Attempt to set role before vehicle externals");
    }
    if (!delete_vehicle_internals.empty()) {
        delete_vehicle_internals.clear();
    }
    InternalsMode new_mode{
        .role = role
    };
    create_vehicle_internals(new_mode);
}

void Player::change_role() {
    std::scoped_lock lock{ mutex_ };
    auto* new_role = rigid_body()->drivers_.next_free_role(internals_mode_.role);
    if (new_role == nullptr) {
        return;
    }
    set_role(*new_role);
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
    return internals_mode_;
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
    if (!dependent_nodes_.try_emplace(node.ptr(), std::move(node_name)).second) {
        THROW_OR_ABORT("Node \"" + *node_name + "\" already is a dependent node of player \"" + id() + '"');
    }
    node->clearing_observers.add({ *this, CURRENT_SOURCE_LOCATION }, ObserverAlreadyExistsBehavior::IGNORE);
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
        Player* player = dynamic_cast<Player*>(iplayer.get());
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
        supply_depots_waypoints_.set_waypoints(wp->way_points);
        ++nfound;
    }
    if (nfound == 0) {
        THROW_OR_ABORT(
            "Player \"" + id_ + "\": Could not find waypoints for final filter \"" +
            joined_way_point_sandbox_to_string(final_filter) + '"');
    }
}
