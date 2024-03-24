#include "Player.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Components/Aim_At.hpp>
#include <Mlib/Components/Gun.hpp>
#include <Mlib/Components/Weapon_Cycle.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Physics/Advance_Times/Bullet.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Aim_At.hpp>
#include <Mlib/Physics/Containers/Collision_Query.hpp>
#include <Mlib/Physics/Containers/Race_Identifier.hpp>
#include <Mlib/Physics/Interfaces/Damageable.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Physics/Misc/Weapon_Cycle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Containers/Vehicle_Spawners.hpp>
#include <Mlib/Players/Scene_Vehicle/Control_Source.hpp>
#include <Mlib/Players/Scene_Vehicle/Externals_Mode.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Players/Scene_Vehicle/Vehicle_Spawner.hpp>
#include <Mlib/Scene_Graph/Animation/Animation_State_Updater.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <fstream>

using namespace Mlib;

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
    const Focuses& focuses)
    : destruction_observers{ *this }
    , car_movement{ *this }
    , avatar_movement{ *this }
    , scene_{ scene }
    , collision_query_{ collision_query }
    , vehicle_spawners_{ vehicle_spawners }
    , players_{ players }
    , name_{ name }
    , team_{ team }
    , vehicle_{ nullptr }
    , controlled_{ .gun_node = nullptr }
    , target_scene_node_{ nullptr }
    , target_rb_{ nullptr }
    , game_mode_{ game_mode }
    , unstuck_mode_{ unstuck_mode }
    , driving_mode_{ driving_mode }
    , driving_direction_{ driving_direction }
    , nunstucked_{ 0 }
    , skills_{
        {ControlSource::AI, Skills{}},
        {ControlSource::USER, Skills{}} }
    , delete_node_mutex_{ delete_node_mutex }
    , next_scene_vehicle_{ nullptr }
    , externals_mode_{ ExternalsMode::NONE }
    , single_waypoint_{ *this, }
    , pathfinding_waypoints_{ *this, cfg }
    , supply_depots_waypoints_{ *this, single_waypoint_, supply_depots }
    , playback_waypoints_{ *this }
    , focuses_{ focuses }
{
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
}

Player::~Player()
{
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    destruction_observers.shutdown();
}

void Player::set_can_drive(ControlSource control_source, bool value) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    skills_.at(control_source).can_drive = value;
}

void Player::set_can_aim(ControlSource control_source, bool value) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    skills_.at(control_source).can_aim = value;
}

void Player::set_can_shoot(ControlSource control_source, bool value) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    skills_.at(control_source).can_shoot = value;
}

void Player::set_can_select_best_weapon(ControlSource control_source, bool value) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    skills_.at(control_source).can_select_best_weapon = value;
}

void Player::reset_node() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (vehicle_ == nullptr) {
        THROW_OR_ABORT("reset_node despite vehicle nullptr");
    }
    if (vehicle_->rb().driver_ != dynamic_cast<IPlayer*>(this)) {
        THROW_OR_ABORT("Rigid body's driver is not player");
    }
    vehicle_->rb().driver_ = nullptr;
    vehicle_->destruction_observers.remove(ref<DestructionObserver<const SceneVehicle&>>(CURRENT_SOURCE_LOCATION));
    vehicle_ = nullptr;
    if (next_scene_vehicle_ != nullptr) {
        next_scene_vehicle_->destruction_observers.remove(ref<DestructionObserver<const SceneVehicle&>>(CURRENT_SOURCE_LOCATION));
        next_scene_vehicle_ = nullptr;
    }
    if (target_scene_node_ != nullptr) {
        target_scene_node_->clearing_observers.remove(ref<DestructionObserver<DanglingRef<SceneNode>>>(CURRENT_SOURCE_LOCATION));
        target_scene_node_ = nullptr;
        target_rb_ = nullptr;
    }
    if (controlled_.gun_node != nullptr) {
        if (controlled_.has_aim_at()) {
            controlled_.aim_at().set_followed(nullptr);
        }
        controlled_.gun_node = nullptr;
    }
    vehicle_movement.reset_node();
    car_movement.reset_node();
    stuck_start_ = std::chrono::steady_clock::time_point();
    unstuck_start_ = std::chrono::steady_clock::time_point();
    if (!delete_externals_.empty()) {
        std::scoped_lock lock{ delete_node_mutex_ };
        clear_map_recursively(delete_externals_, [](const auto& p){
            p.key() = nullptr;
            p.mapped()();
            });
    }
    if (!dependent_nodes_.empty()) {
        std::scoped_lock lock{ delete_node_mutex_ };
        clear_map_recursively(dependent_nodes_, [this](const auto& p){
            p.key() = nullptr;
            scene_.delete_node(p.mapped());
            });
    }
    externals_mode_ = ExternalsMode::NONE;
}

void Player::set_scene_vehicle(SceneVehicle& pv) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (vehicle_ != nullptr) {
        THROW_OR_ABORT("Scene vehicle already set");
    }
    vehicle_ = &pv;
    pv.rb().driver_ = this;
    vehicle_->destruction_observers.add(ref<DestructionObserver<const SceneVehicle&>>(CURRENT_SOURCE_LOCATION));
}

RigidBodyVehicle& Player::rigid_body() {
    const Player* cthis = this;
    return const_cast<RigidBodyVehicle&>(cthis->rigid_body());
}

const RigidBodyVehicle& Player::rigid_body() const {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!has_scene_vehicle()) {
        THROW_OR_ABORT("Player has no rigid body");
    }
    return vehicle_->rb();
}

const std::string& Player::scene_node_name() const {
    delete_node_mutex_.notify_reading();
    return vehicle().scene_node_name();
}

void Player::set_gun_node(DanglingRef<SceneNode> gun_node) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (controlled_.gun_node != nullptr) {
        THROW_OR_ABORT("gun already set");
    }
    controlled_.gun_node = gun_node.ptr();
}

const std::string& Player::name() const {
    delete_node_mutex_.notify_reading();
    return name_;
}

const std::string& Player::team_name() const {
    delete_node_mutex_.notify_reading();
    return team_;
}

Team& Player::team() {
    delete_node_mutex_.notify_reading();
    return players_.get_team(team_name());
}

PlayerStats& Player::stats() {
    delete_node_mutex_.notify_reading();
    return stats_;
}

const PlayerStats& Player::stats() const {
    delete_node_mutex_.notify_reading();
    return stats_;
}

float Player::car_health() const {
    delete_node_mutex_.notify_reading();
    if (has_scene_vehicle() && (vehicle_->rb().damageable_ != nullptr)) {
        return vehicle_->rb().damageable_->health();
    } else {
        return NAN;
    }
}

std::string Player::vehicle_name() const {
    delete_node_mutex_.notify_reading();
    if (!has_scene_vehicle()) {
        THROW_OR_ABORT("Player has no scene vehicle, cannot get vehicle name");
    }
    return vehicle_->rb().name();
}

GameMode Player::game_mode() const {
    delete_node_mutex_.notify_reading();
    return game_mode_;
}

bool Player::can_see(
    const RigidBodyVehicle& rb,
    bool only_terrain,
    float height_offset,
    float time_offset) const
{
    delete_node_mutex_.notify_reading();
    if (!has_scene_vehicle()) {
        THROW_OR_ABORT("Player::can_see requires rb");
    }
    return collision_query_.can_see(
        vehicle_->rb(),
        rb,
        only_terrain,
        PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK,
        height_offset,
        time_offset);
}

bool Player::can_see(
    const FixedArray<double, 3>& pos,
    bool only_terrain,
    float height_offset,
    float time_offset) const
{
    delete_node_mutex_.notify_reading();
    if (!has_scene_vehicle()) {
        THROW_OR_ABORT("Player::can_see requires rb");
    }
    return collision_query_.can_see(
        vehicle_->rb(),
        pos,
        only_terrain,
        PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK,
        height_offset,
        time_offset);
}

bool Player::can_see(
    const SceneVehicle& scene_vehicle,
    bool only_terrain,
    float height_offset,
    float time_offset) const
{
    delete_node_mutex_.notify_reading();
    if (!has_scene_vehicle()) {
        THROW_OR_ABORT("Player::can_see requires vehicle");
    }
    return collision_query_.can_see(
        vehicle_->rb(),
        scene_vehicle.rb(),
        only_terrain,
        PhysicsMaterial::OBJ_BULLET_COLLIDABLE_MASK,
        height_offset,
        time_offset);
}

bool Player::can_see(
    const Player& player,
    bool only_terrain,
    float height_offset,
    float time_offset) const
{
    delete_node_mutex_.notify_reading();
    if (!player.has_scene_vehicle()) {
        THROW_OR_ABORT("Player::can_see requires target rb");
    }
    return can_see(
        player.vehicle(),
        only_terrain,
        height_offset,
        time_offset);
}

void Player::notify_destroyed(DanglingRef<SceneNode> destroyed_object) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (destroyed_object.ptr() == target_scene_node_) {
        target_scene_node_ = nullptr;
        target_rb_ = nullptr;
    }
    // The node has already removed the player from its observers,
    // so nothing no deregistration is done here.
    dependent_nodes_.erase(destroyed_object.ptr());
}

void Player::notify_destroyed(const SceneVehicle& destroyed_object) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (&destroyed_object == next_scene_vehicle_) {
        next_scene_vehicle_ = nullptr;
    }
    if (&destroyed_object == vehicle_) {
        reset_node();
    } 
}

void Player::advance_time(float dt) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    aim_and_shoot();
    select_best_weapon_in_inventory();
}

void Player::increment_external_forces(
    const std::list<RigidBodyVehicle*>& olist,
    bool burn_in,
    const PhysicsEngineConfig& cfg)
{
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
    if (has_scene_vehicle()) {
        bool countdown_active;
        {
            std::shared_lock lock{focuses_.mutex};
            countdown_active = focuses_.countdown_active();
        }
        if (countdown_active) {
            car_movement.step_on_brakes();
            car_movement.steer(0.f);
            vehicle_->rb().vehicle_controller().apply();
            return;
        }
    }
    if (game_mode_ == GameMode::RACING) {
        if (playback_waypoints_.has_waypoints()) {
            playback_waypoints_.select_next_waypoint();
        }
    } else if ((unstuck_mode_ == UnstuckMode::OFF) || !unstuck()) {
        if (ramming()) {
            auto tpos = target_rb_->abs_target();
            single_waypoint_.set_waypoint(tpos);
        } else {
            if (!supply_depots_waypoints_.select_next_waypoint()) {
                pathfinding_waypoints_.select_next_waypoint();
            }
        }
    }
    single_waypoint_.move_to_waypoint();
}

bool Player::unstuck() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!has_scene_vehicle()) {
        return false;
    }
    if ((sum(squared(vehicle_->rb().rbp_.v_)) > squared(driving_mode_.stuck_velocity)) ||
        (unstuck_start_ != std::chrono::steady_clock::time_point()))
    {
        stuck_start_ = std::chrono::steady_clock::now();
    } else if (
        (stuck_start_ != std::chrono::steady_clock::time_point()) &&
        (unstuck_start_ == std::chrono::steady_clock::time_point()) &&
        std::chrono::duration<float>(std::chrono::steady_clock::now() - stuck_start_).count() > driving_mode_.stuck_seconds)
    {
        unstuck_start_ = std::chrono::steady_clock::now();
    }
    if (unstuck_start_ != std::chrono::steady_clock::time_point()) {
        if (std::chrono::duration<float>(std::chrono::steady_clock::now() - unstuck_start_).count() > driving_mode_.unstuck_seconds)
        {
            unstuck_start_ = std::chrono::steady_clock::time_point();
        } else {
            // if (!waypoint_history_.empty()) {
            //     draw_waypoint_history("/tmp/" + name() + "_" + std::to_string(nunstucked_++) + ".svg");
            // }
            if (unstuck_mode_ == UnstuckMode::REVERSE) {
                car_movement.drive_backwards();
                vehicle_->rb().vehicle_controller().steer(0, 1.f);
                vehicle_->rb().vehicle_controller().apply();
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
    delete_node_mutex_.notify_reading();
    if (controlled_.gun_node == nullptr) {
        THROW_OR_ABORT("gun_direction despite gun nullptr in player \"" + name() + '"');
    }
    return -z3_from_3x3(gun().absolute_model_matrix().R());
}

FixedArray<float, 3> Player::punch_angle() const {
    delete_node_mutex_.notify_reading();
    if (controlled_.gun_node == nullptr) {
        THROW_OR_ABORT("punch_angle despite gun nullptr in player \"" + name() + '"');
    }
    return gun().punch_angle();
}

void Player::trigger_gun() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (controlled_.gun_node == nullptr) {
        THROW_OR_ABORT("Player::trigger despite gun nullptr");
    }
    gun().trigger(this, &team());
}

bool Player::has_gun_node() const {
    return (controlled_.gun_node != nullptr);
}

Inventory& Player::inventory() {
    return rigid_body().inventory_;
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
    if (!has_scene_vehicle()) {
        return false;
    }
    return nbullets_available() == 0;
}

size_t Player::nbullets_available() const {
    return gun().nbullets_available();
}

std::string Player::best_weapon_in_inventory() const {
    auto& wc = weapon_cycle();
    if ((target_rb_ == nullptr) ||
        !has_scene_vehicle())
    {
        return "";
    }
    const auto& inventy = inventory();
    double distance_to_target = std::sqrt(sum(squared(
        target_rb_->rbp_.abs_position() - rigid_body().rbp_.abs_position())));
    float best_score = -INFINITY;
    std::string best_weapon_name;
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
    delete_node_mutex_.notify_reading();
    if (vehicle_ == nullptr) {
        return false;
    }
    if (vehicle_->scene_node()->shutting_down()) {
        verbose_abort("Player::has_rigid_body: Scene node shutting down");
    }
    if (scene_.root_node_scheduled_for_deletion(vehicle_->scene_node_name())) {
        return false;
    }
    return true;
}

bool Player::has_vehicle_controller() const {
    return rigid_body().has_vehicle_controller();
}

const Gun& Player::gun() const {
    if (controlled_.gun_node == nullptr) {
        THROW_OR_ABORT("Gun node not set");
    }
    return get_gun(*controlled_.gun_node);
}

Gun& Player::gun() {
    const Player* p = this;
    return const_cast<Gun&>(p->gun());
}

bool Player::is_pedestrian() const {
    delete_node_mutex_.notify_reading();
    return driving_mode_.way_point_location == WayPointLocation::SIDEWALK;
}

void Player::aim_and_shoot() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!skills_.at(ControlSource::AI).can_aim) {
        return;
    }
    assert_true((target_scene_node_ == nullptr) == (target_rb_ == nullptr));
    if ((target_rb_ != nullptr) && !can_see(*target_rb_)) {
        clear_opponent();
    }
    if (target_rb_ == nullptr) {
        select_next_opponent();
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
    if (!skills_.at(ControlSource::AI).can_shoot) {
        return;
    }
    if ((target_scene_node_ != nullptr) && (controlled_.aim_at().target_locked_on())) {
        gun().trigger(this, &team());
    }
}

void Player::select_best_weapon_in_inventory() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!skills_.at(ControlSource::AI).can_select_best_weapon) {
        return;
    }
    if (!has_scene_vehicle()) {
        return;
    }
    if (!has_weapon_cycle()) {
        return;
    }
    std::string best_weapon = best_weapon_in_inventory();
    if (best_weapon.empty()) {
        return;
    }
    weapon_cycle().set_desired_weapon(best_weapon);
}

bool Player::ramming() const {
    delete_node_mutex_.notify_reading();
    return (game_mode_ == GameMode::RAMMING) && (target_rb_ != nullptr);
}

void Player::select_next_opponent() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!has_scene_vehicle()) {
        return;
    }
    if (players_.players().empty()) {
        THROW_OR_ABORT("List of players is empty");
    }
    size_t current_opponent_index = SIZE_MAX;
    std::vector<const Player*> players_vec;
    players_vec.reserve(players_.players().size());
    for (const auto& [_, p] : players_.players()) {
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
    size_t i = current_opponent_index;
    while (true) {
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
        const Player& p = *players_vec[i];
        if (p.team_ != team_) {
            if (!p.has_scene_vehicle()) {
                continue;
            }
            if (can_see(p.vehicle_->rb())) {
                if (target_scene_node_ != nullptr) {
                    clear_opponent();
                }
                set_opponent(p);
                break;
            }
        }
    }
}

void Player::clear_opponent() {
    if (target_scene_node_ == nullptr) {
        THROW_OR_ABORT("Player has no opponent");
    }
    target_scene_node_->clearing_observers.remove(ref<DestructionObserver<DanglingRef<SceneNode>>>(CURRENT_SOURCE_LOCATION));
    target_scene_node_ = nullptr;
    target_rb_ = nullptr;
}

void Player::set_opponent(const Player& opponent) {
    if (target_scene_node_ != nullptr) {
        THROW_OR_ABORT("Player already has an opponent");
    }
    if (opponent.vehicle_ == nullptr) {
        THROW_OR_ABORT("Opponent has no avatar or vehicle");
    }
    target_scene_node_ = opponent.vehicle_->scene_node().ptr();
    target_rb_ = &opponent.vehicle_->rb();
    target_scene_node_->clearing_observers.add(ref<DestructionObserver<DanglingRef<SceneNode>>>(CURRENT_SOURCE_LOCATION));
}

DanglingRef<SceneNode> Player::scene_node() {
    if (!has_scene_vehicle()) {
        THROW_OR_ABORT("Player has no scene node");
    }
    return vehicle_->scene_node();
}

DanglingRef<const SceneNode> Player::scene_node() const {
    return const_cast<Player*>(this)->scene_node();
}

SceneVehicle* Player::next_scene_vehicle() {
    return next_scene_vehicle_;
}

SceneVehicle& Player::vehicle() {
    return const_cast<SceneVehicle&>(static_cast<const Player*>(this)->vehicle());
}

const SceneVehicle& Player::vehicle() const {
    if (vehicle_ == nullptr) {
        THROW_OR_ABORT("Vehicle is null");
    }
    return *vehicle_;
}

void Player::select_next_vehicle() {
    if (!has_scene_vehicle()) {
        return;
    }
    double closest_distance2 = INFINITY;
    auto clear_next_scene_vehicle = [this](){
        if (next_scene_vehicle_ != nullptr) {
            next_scene_vehicle_->destruction_observers.remove(ref<DestructionObserver<const SceneVehicle&>>(CURRENT_SOURCE_LOCATION));
            next_scene_vehicle_ = nullptr;
        }
    };
    clear_next_scene_vehicle();
    for (auto& [_, s] : vehicle_spawners_.spawners()) {
        if (!s->has_scene_vehicle()) {
            continue;
        }
        auto& v = s->get_primary_scene_vehicle();
        if (vehicle_ == &v) {
            continue;
        }
        if (v.rb().driver_ != nullptr) {
            continue;
        }
        auto set_next_scene_vehicle = [&](){
            clear_next_scene_vehicle();
            next_scene_vehicle_ = &v;
            v.destruction_observers.add(ref<DestructionObserver<const SceneVehicle&>>(CURRENT_SOURCE_LOCATION));
        };
        if (s->has_player() && (&s->get_player() == this)) {
            set_next_scene_vehicle();
            break;
        }
        double dist2 = sum(squared(v.rb().rbp_.abs_position() - vehicle_->rb().rbp_.abs_position()));
        if (dist2 < closest_distance2) {
            set_next_scene_vehicle();
            closest_distance2 = dist2;
        }
    }
}

void Player::create_externals(ExternalsMode externals_mode) {
    if (externals_mode_ != ExternalsMode::NONE) {
        THROW_OR_ABORT("Externals already created (0)");
    }
    if (!has_scene_vehicle()) {
        THROW_OR_ABORT("Create_externals without scene vehicle");
    }
    vehicle_->create_externals(name(), externals_mode, skills_);
    externals_mode_ = externals_mode;
}

ExternalsMode Player::externals_mode() const {
    return externals_mode_;
}

SingleWaypoint& Player::single_waypoint() {
    return single_waypoint_;
}

PathfindingWaypoints& Player::pathfinding_waypoints() {
    if (game_mode_ == GameMode::RACING) {
        THROW_OR_ABORT("Player::pathfinding_waypoints called, but game mode is racing");
    }
    return pathfinding_waypoints_;
}

PlaybackWaypoints& Player::playback_waypoints() {
    if (game_mode_ != GameMode::RACING) {
        THROW_OR_ABORT("Player::playback_waypoints called, but game mode is not racing");
    }
    return playback_waypoints_;
}

void Player::append_delete_externals(
    DanglingPtr<SceneNode> node,
    const std::function<void()>& delete_externals)
{
    // Consider reading the line
    // "delete_externals_.erase((SceneNode*)destroyed_object);"
    // in "Player::notify_destroyed" and the comments above it.
    delete_externals_.insert({ node, delete_externals });
    if (node != nullptr) {
        node->clearing_observers.add(ref<DestructionObserver<DanglingRef<SceneNode>>>(CURRENT_SOURCE_LOCATION), ObserverAlreadyExistsBehavior::IGNORE);
    }
}

void Player::append_dependent_node(std::string node_name) {
    auto node = scene_.get_node(node_name, DP_LOC);
    if (!dependent_nodes_.try_emplace(node.ptr(), std::move(node_name)).second) {
        THROW_OR_ABORT("Node \"" + node_name + "\" already is a dependent node of player \"" + name() + '"');
    }
    node->clearing_observers.add(ref<DestructionObserver<DanglingRef<SceneNode>>>(CURRENT_SOURCE_LOCATION), ObserverAlreadyExistsBehavior::IGNORE);
}

void Player::notify_race_started() {
    stats_ = PlayerStats();
}

RaceState Player::notify_lap_finished(
    float race_time_seconds,
    const std::string& asset_id,
    const std::vector<FixedArray<float, 3>>& vehicle_colors,
    const std::list<float>& lap_times_seconds,
    const std::list<TrackElement>& track)
{
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

void Player::notify_vehicle_destroyed() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    reset_node();
}

void Player::notify_kill(RigidBodyVehicle& rigid_body_vehicle) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (rigid_body_vehicle.driver_ == nullptr) {
        return;
    }
    Player* player = dynamic_cast<Player*>(rigid_body_vehicle.driver_);
    if (player == nullptr) {
        THROW_OR_ABORT("Driver is not a player");
    }
    if (player->team_name() != team_name()) {
        ++stats_.nkills;
    }
}

void Player::notify_bullet_destroyed(Bullet& bullet) {
    destruction_observers.remove(bullet.ref<DestructionObserver<const IPlayer&>>(CURRENT_SOURCE_LOCATION));
}

void Player::set_pathfinding_waypoints(
    const std::map<WayPointLocation, PointsAndAdjacency<double, 3>>& way_points)
{
    auto it = way_points.find(driving_mode_.way_point_location);
    if (it == way_points.end()) {
        THROW_OR_ABORT(
            "Could not find waypoints for location \"" +
            way_point_location_to_string(driving_mode_.way_point_location) +
            '"');
    }
    pathfinding_waypoints_.set_waypoints(it->second);
    supply_depots_waypoints_.set_waypoints(it->second);
}
