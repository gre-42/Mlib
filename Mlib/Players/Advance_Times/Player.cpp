#include "Player.hpp"
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Math/Signed_Min.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Containers/Collision_Query.hpp>
#include <Mlib/Physics/Interfaces/Damageable.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Mlib_Pod_Bot/Pod_Bot_Player.hpp>
#include <Mlib/Players/Pod_Bot_Mlib_Compat/mlib.hpp>
#include <Mlib/Recursive_Deletion.hpp>
#include <Mlib/Scene_Graph/Animation_State_Updater.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Elements/Color_Style.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>
#include <fstream>

using namespace Mlib;

Player::Player(
    Scene& scene,
    CollisionQuery& collision_query,
    Players& players,
    const std::string& name,
    const std::string& team,
    GameMode game_mode,
    UnstuckMode unstuck_mode,
    const DrivingMode& driving_mode,
    DrivingDirection driving_direction,
    DeleteNodeMutex& delete_node_mutex)
: scene_{ scene },
  collision_query_{ collision_query },
  players_{ players },
  name_{ name },
  team_{ team },
  vehicle_{
      .scene_node{ nullptr },
      .rb{ nullptr }
  },
  controlled_{
      .ypln{ nullptr },
      .gun_node{ nullptr }
  },
  target_scene_node_{ nullptr },
  target_rb_{ nullptr },
  surface_power_forward_{ NAN },
  surface_power_backward_{ NAN },
  max_tire_angle_{ NAN },
  tire_angle_pid_{ NAN, NAN, NAN, NAN },
  game_mode_{ game_mode },
  unstuck_mode_{ unstuck_mode },
  driving_mode_{ driving_mode },
  driving_direction_{ driving_direction },
  spotted_by_vip_{ false },
  nunstucked_{ 0 },
  skills_{
    {ControlSource::AI, Skills{}},
    {ControlSource::USER, Skills{}}},
  delete_node_mutex_{ delete_node_mutex },
  next_scene_node_{ nullptr },
  externals_mode_{ ExternalsMode::NONE },
  single_waypoint_{ *this },
  pathfinding_waypoints_{ *this },
  playback_waypoints_{ *this }
{
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if ((game_mode_ == GameMode::POD_BOT_NPC) || (game_mode_ == GameMode::POD_BOT_PC)) {
        pod_bot_player_ = std::make_unique<PodBotPlayer>(*this);
    }
}

Player::~Player()
{
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    pod_bot_player_.reset();
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

void Player::reset_node() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (vehicle_.rb == nullptr) {
        throw std::runtime_error("reset_node despite rb nullptr");
    }
    if (vehicle_.scene_node == nullptr) {
        throw std::runtime_error("reset_node despite node nullptr");
    }
    if ((vehicle_.rb != nullptr) && (pod_bot_player_ != nullptr)) {
        pod_bot_player_->clear_rigid_body_integrator();
    }
    if (vehicle_.rb->driver_ != dynamic_cast<IPlayer*>(this)) {
        throw std::runtime_error("Rigid body's driver is not player");
    }
    vehicle_.rb->driver_ = nullptr;
    vehicle_.scene_node->remove_destruction_observer(this, true);
    vehicle_.scene_node_name.clear();
    vehicle_.scene_node = nullptr;
    vehicle_.rb = nullptr;
    controlled_.gun_node = nullptr;
    if (next_scene_node_ != nullptr) {
        next_scene_node_->remove_destruction_observer(this);
        next_scene_node_ = nullptr;
    }
    if (target_scene_node_ != nullptr) {
        target_scene_node_->remove_destruction_observer(this);
        target_scene_node_ = nullptr;
        target_rb_ = nullptr;
        if (controlled_.ypln != nullptr) {
            controlled_.ypln->set_followed(nullptr, nullptr);
        }
    }
    controlled_.ypln = nullptr;
    surface_power_forward_ = NAN;
    surface_power_backward_ = NAN;
    max_tire_angle_ = NAN;
    tire_angle_pid_ = PidController<float, float>{NAN, NAN, NAN, NAN};
    stuck_start_ = std::chrono::steady_clock::time_point();
    unstuck_start_ = std::chrono::steady_clock::time_point();
    if (!delete_externals_.empty()) {
        std::lock_guard lock{ delete_node_mutex_ };
        clear_map_recursively(delete_externals_, [](const auto& p){
            p.mapped()();
        });
    }
    externals_mode_ = ExternalsMode::NONE;
}

void Player::set_rigid_body(const PlayerVehicle& pv) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (pv.rb == nullptr) {
        throw std::runtime_error("Rigid body is null");
    }
    if (pv.scene_node_name.empty()) {
        throw std::runtime_error("Rigid body scene node name is empty");
    }
    if (pv.scene_node == nullptr) {
        throw std::runtime_error("Rigid body scene node is null");
    }
    if (pv.rb->driver_ != nullptr) {
        throw std::runtime_error("Rigid body already has a driver");
    }
    if ((vehicle_.scene_node != nullptr) || (vehicle_.rb != nullptr)) {
        throw std::runtime_error("Player scene node or rb already set");
    }
    if (pv.scene_node->shutting_down()) {
        throw std::runtime_error("Player received scene node that is shutting down");
    }
    if (scene_.root_node_scheduled_for_deletion(pv.scene_node_name)) {
        throw std::runtime_error("Player received root node scheduled for deletion");
    }
    assert_true(pv.rb->driver_ == nullptr);
    assert_true(vehicle_.rb == nullptr);
    vehicle_ = pv;
    pv.rb->driver_ = this;
    if (pod_bot_player_ != nullptr) {
        pod_bot_player_->set_rigid_body_integrator();
    }
}

const RigidBodyVehicle& Player::rigid_body() const {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!has_rigid_body()) {
        throw std::runtime_error("Player has no rigid body");
    }
    return *vehicle_.rb;
}

const std::string& Player::scene_node_name() const {
    delete_node_mutex_.notify_reading();
    return vehicle_.scene_node_name;
}

void Player::set_ypln(YawPitchLookAtNodes& ypln, SceneNode* gun_node) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (controlled_.ypln != nullptr) {
        throw std::runtime_error("ypln already set");
    }
    if (controlled_.gun_node != nullptr) {
        throw std::runtime_error("gun already set");
    }
    controlled_.ypln = &ypln;
    controlled_.gun_node = gun_node;
}

void Player::set_vehicle_control_parameters(
    float surface_power_forward,
    float surface_power_backward,
    float max_tire_angle,
    const PidController<float, float>& tire_angle_pid)
{
    if (!std::isnan(surface_power_forward_)) {
        throw std::runtime_error("surface_power_forward already set");
    }
    surface_power_forward_ = surface_power_forward;
    if (!std::isnan(surface_power_backward_)) {
        throw std::runtime_error("surface_power_backward already set");
    }
    surface_power_backward_ = surface_power_backward;
    if (!std::isnan(max_tire_angle_)) {
        throw std::runtime_error("max_tire_angle_ already set");
    }
    max_tire_angle_ = max_tire_angle;
    tire_angle_pid_ = tire_angle_pid;
}

const std::string& Player::name() const {
    delete_node_mutex_.notify_reading();
    return name_;
}

const std::string& Player::team() const {
    delete_node_mutex_.notify_reading();
    return team_;
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
    if (has_rigid_body() && (vehicle_.rb->damageable_ != nullptr)) {
        return vehicle_.rb->damageable_->health();
    } else {
        return NAN;
    }
}

std::string Player::vehicle_name() const {
    delete_node_mutex_.notify_reading();
    // return has_rigid_body() ? vehicle_.rb->name() : "";
    if (!has_rigid_body()) {
        throw std::runtime_error("Player has no rigid body, cannot get vehicle name");
    }
    return vehicle_.rb->name();
}

FixedArray<float, 3> Player::vehicle_color() const {
    delete_node_mutex_.notify_reading();
    if (vehicle_.scene_node == nullptr) {
        throw std::runtime_error("Player has no scene node, cannot get vehicle color");
    }
    const std::string chassis = "chassis";
    if (!vehicle_.scene_node->has_color_style(chassis)) {
        return FixedArray<float, 3>(1.f, 1.f, 1.f);
    }
    const auto& style = vehicle_.scene_node->color_style(chassis);
    if (!all(style.ambience == style.diffusivity)) {
        throw std::runtime_error("Could not determine unique vehicle color");
    }
    return style.ambience;
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
    if (!has_rigid_body()) {
        throw std::runtime_error("Player::can_see requires rb");
    }
    return collision_query_.can_see(
        *vehicle_.rb,
        rb,
        only_terrain,
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
    if (!has_rigid_body()) {
        throw std::runtime_error("Player::can_see requires rb");
    }
    return collision_query_.can_see(
        *vehicle_.rb,
        pos,
        only_terrain,
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
    if (!has_rigid_body()) {
        throw std::runtime_error("Player::can_see requires rb");
    }
    if (!player.has_rigid_body()) {
        throw std::runtime_error("Player::can_see requires target rb");
    }
    return collision_query_.can_see(
        *vehicle_.rb,
        *player.vehicle_.rb,
        only_terrain,
        height_offset,
        time_offset);
}

void Player::notify_spawn() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    single_waypoint_.notify_spawn();
    spawn_time_ = std::chrono::steady_clock::now();
    spotted_by_vip_ = false;
}

float Player::seconds_since_spawn() const {
    delete_node_mutex_.notify_reading();
    if (spawn_time_ == std::chrono::time_point<std::chrono::steady_clock>()) {
        throw std::runtime_error("Seconds since spawn requires previous call to notify_spawn");
    }
    return std::chrono::duration<float>(std::chrono::steady_clock::now() - spawn_time_).count();
}

bool Player::spotted_by_vip() const {
    delete_node_mutex_.notify_reading();
    return spotted_by_vip_;
}
void Player::set_spotted_by_vip() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    spotted_by_vip_ = true;
}

void Player::notify_destroyed(void* destroyed_object) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (destroyed_object == target_scene_node_) {
        target_scene_node_ = nullptr;
        target_rb_ = nullptr;
    }
    if (destroyed_object == next_scene_node_) {
        next_scene_node_ = nullptr;
    }
    // If node != nullptr in "append_delete_externals",
    // it is assumed that all objects that would be
    // deleted in the externals-deleters are children of
    // the external nodes. The children will therefore get
    // deleted by the node itself.
    delete_externals_.erase((SceneNode*)destroyed_object);
}

void Player::advance_time(float dt) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    aim_and_shoot();
}

void Player::increment_external_forces(
    const std::list<std::shared_ptr<RigidBodyVehicle>>& olist,
    bool burn_in,
    const PhysicsEngineConfig& cfg)
{
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!burn_in) {
        if (game_mode_ == GameMode::POD_BOT_NPC) {
            // Do nothing, is handled by PodBots
            // std::cerr << "Game mode pod_bot_npc not yet implemented" << std::endl;
        } else {
            if (game_mode_ == GameMode::RACING) {
                if (playback_waypoints_.has_waypoints()) {
                    playback_waypoints_.select_next_waypoint();
                }
            } else if ((unstuck_mode_ == UnstuckMode::OFF) || !unstuck()) {
                if (ramming()) {
                    auto tpos = target_rb_->abs_target();
                    single_waypoint_.set_waypoint(tpos);
                } else {
                    if (pathfinding_waypoints_.has_waypoints()) {
                        pathfinding_waypoints_.select_next_waypoint();
                    }
                }
            }
            single_waypoint_.move_to_waypoint();
        }
    }
}

bool Player::unstuck() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!has_rigid_body()) {
        return false;
    }
    if ((sum(squared(vehicle_.rb->rbi_.rbp_.v_)) > squared(driving_mode_.stuck_velocity)) ||
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
                drive_backwards();
                vehicle_.rb->vehicle_controller().steer(0);
                vehicle_.rb->vehicle_controller().apply();
            } else if (unstuck_mode_ == UnstuckMode::DELETE) {
                // std::lock_guard lock{ mutex_ };
                // scene_.delete_root_node(vehicle_.scene_node_name);
                scene_.schedule_delete_root_node(vehicle_.scene_node_name);
            } else {
                throw std::runtime_error("Unsupported unstuck mode");
            }
            return true;
        }
    }
    return false;
}

FixedArray<float, 3> Player::gun_direction() const {
    delete_node_mutex_.notify_reading();
    if (controlled_.gun_node == nullptr) {
        throw std::runtime_error("gun_direction despite gun nullptr in player \"" + name() + '"');
    }
    return -z3_from_3x3(gun()->absolute_model_matrix().R());
}

FixedArray<float, 3> Player::punch_angle() const {
    delete_node_mutex_.notify_reading();
    if (controlled_.gun_node == nullptr) {
        throw std::runtime_error("punch_angle despite gun nullptr in player \"" + name() + '"');
    }
    return gun()->punch_angle();
}

void Player::run_move(
    float yaw,
    float pitch,
    float forwardmove,
    float sidemove)
{
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!has_rigid_body()) {
        throw std::runtime_error("run_move despite rigid body nullptr");
    }
    if (controlled_.ypln == nullptr) {
        throw std::runtime_error("run_move despite ypln nullptr");
    }

    controlled_.ypln->set_yaw(yaw);
    controlled_.ypln->pitch_look_at_node()->set_pitch(pitch);

    FixedArray<float, 3> direction{ sidemove, 0.f, -forwardmove };
    float len2 = sum(squared(direction));
    if (len2 < 1e-12) {
        step_on_brakes();
    } else {
        float len = std::sqrt(len2);
        vehicle_.rb->tires_z_ = direction / len;
        vehicle_.rb->set_surface_power("legs", len * surface_power_forward_);
    }
    if (vehicle_.rb->animation_state_updater_ != nullptr) {
        vehicle_.rb->animation_state_updater_->notify_movement_intent();
    }
}

void Player::trigger_gun() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (controlled_.gun_node == nullptr) {
        throw std::runtime_error("Player::trigger despite gun nullptr");
    }
    gun()->trigger();
}

void Player::step_on_brakes() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!has_rigid_body()) {
        throw std::runtime_error("step_on_brakes despite nullptr");
    }
    vehicle_.rb->vehicle_controller().step_on_brakes();
}

void Player::drive_forward() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!has_rigid_body()) {
        throw std::runtime_error("drive_forward despite nullptr");
    }
    vehicle_.rb->vehicle_controller().drive(surface_power_forward_);
}

void Player::drive_backwards() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!has_rigid_body()) {
        throw std::runtime_error("drive_backwards despite nullptr");
    }
    vehicle_.rb->vehicle_controller().drive(surface_power_backward_);
}

void Player::roll_tires() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!has_rigid_body()) {
        throw std::runtime_error("roll despite nullptr");
    }
    vehicle_.rb->vehicle_controller().roll_tires();
}

void Player::steer(float angle) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    vehicle_.rb->vehicle_controller().steer(tire_angle_pid_(signed_min(angle, max_tire_angle_)));
}

void Player::steer_left_full() {
    steer(INFINITY);
}

void Player::steer_right_full() {
    steer(-INFINITY);
}

void Player::steer_left_partial(float angle) {
    steer(angle);
}

void Player::steer_right_partial(float angle) {
    steer(-angle);
}

bool Player::has_rigid_body() const {
    delete_node_mutex_.notify_reading();
    if (vehicle_.rb == nullptr) {
        return false;
    }
    if (scene_node().shutting_down()) {
        throw std::runtime_error("Player::has_rigid_body: Scene node shutting down");
    }
    if (scene_.root_node_scheduled_for_deletion(vehicle_.scene_node_name))
    {
        return false;
    }
    return true;
}

const Gun* Player::gun() const {
    if (controlled_.gun_node == nullptr) {
        throw std::runtime_error("Gun node not set");
    }
    Gun* gun = dynamic_cast<Gun*>(&controlled_.gun_node->get_absolute_observer());
    if (gun == nullptr) {
        throw std::runtime_error("Absolute observer is not a gun");
    }
    return gun;
}

Gun* Player::gun() {
    const Player* p = this;
    return const_cast<Gun*>(p->gun());
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
    if (game_mode_ == GameMode::POD_BOT_NPC) {
        return;
    }
    assert_true(!target_scene_node_ == !target_rb_);
    if (has_rigid_body() && ((target_rb_ == nullptr) || !can_see(*target_rb_))) {
        select_next_opponent();
    }
    if (controlled_.ypln == nullptr) {
        return;
    }
    assert_true(vehicle_.scene_node == nullptr || vehicle_.scene_node != target_scene_node_);
    controlled_.ypln->set_followed(target_scene_node_, target_rb_);
    if (controlled_.gun_node == nullptr) {
        return;
    }
    if (!skills_.at(ControlSource::AI).can_shoot) {
        return;
    }
    if ((target_scene_node_ != nullptr) && (controlled_.ypln->target_locked_on())) {
        gun()->trigger();
    }
}

bool Player::ramming() const {
    delete_node_mutex_.notify_reading();
    return (game_mode_ == GameMode::RAMMING) && (target_rb_ != nullptr);
}

void Player::select_next_opponent() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    assert_true(!vehicle_.scene_node == !vehicle_.rb);
    if (!has_rigid_body()) {
        return;
    }
    if (players_.players().empty()) {
        throw std::runtime_error("List of players is empty");
    }
    size_t current_opponent_index = SIZE_MAX;
    std::vector<const Player*> players_vec;
    players_vec.reserve(players_.players().size());
    for (const auto& [_, p] : players_.players()) {
        if ((target_scene_node_ != nullptr) &&
            (target_scene_node_ == p->vehicle_.scene_node))
        {
            if (current_opponent_index != SIZE_MAX) {
                throw std::runtime_error("Found multiple players with target node");
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
            assert_true(!p.vehicle_.scene_node == !p.vehicle_.rb);
            if (!p.has_rigid_body()) {
                continue;
            }
            if (can_see(*p.vehicle_.rb)) {
                if (target_scene_node_ != nullptr) {
                    target_scene_node_->remove_destruction_observer(this);
                    target_scene_node_ = nullptr;
                    target_rb_ = nullptr;
                }
                target_scene_node_ = p.vehicle_.scene_node;
                target_rb_ = p.vehicle_.rb;
                target_scene_node_->add_destruction_observer(this);
                break;
            }
        }
    }
}

bool Player::has_scene_node() const {
    return (vehicle_.scene_node != nullptr);
}

SceneNode& Player::scene_node() {
    if (!has_scene_node()) {
        throw std::runtime_error("Player has no scene node");
    }
    return *vehicle_.scene_node;
}

const SceneNode& Player::scene_node() const {
    return const_cast<Player*>(this)->scene_node();
}

const SceneNode* Player::next_scene_node() const {
    return next_scene_node_;
}

const PlayerVehicle& Player::vehicle() const {
    return vehicle_;
}

void Player::select_next_vehicle() {
    if (vehicle_.rb == nullptr) {
        return;
    }
    float closest_distance2 = INFINITY;
    if (next_scene_node_ != nullptr) {
        next_scene_node_->remove_destruction_observer(this);
        next_scene_node_ = nullptr;
    }
    for (const auto& [_, p] : players_.players()) {
        if (vehicle_.scene_node == p->vehicle_.scene_node) {
            continue;
        }
        if (p->game_mode() != GameMode::BYSTANDER) {
            continue;
        }
        if (p->vehicle_.rb == nullptr) {
            continue;
        }
        float dist2 = sum(squared(p->vehicle_.rb->rbi_.abs_position() - vehicle_.rb->rbi_.abs_position()));
        if (dist2 < closest_distance2) {
            if (next_scene_node_ != nullptr) {
                next_scene_node_->remove_destruction_observer(this);
                next_scene_node_ = nullptr;
            }
            next_scene_node_ = p->vehicle_.scene_node;
            p->vehicle_.scene_node->add_destruction_observer(this);
            closest_distance2 = dist2;
        }
    }
}

void Player::create_externals(ExternalsMode externals_mode) {
    if (externals_mode_ != ExternalsMode::NONE) {
        throw std::runtime_error("Externals already created (0)");
    }
    if (!vehicle_.create_externals) {
        throw std::runtime_error("create_externals not set");
    }
    vehicle_.create_externals(name(), externals_mode, skills_);
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
        throw std::runtime_error("Player::pathfinding_waypoints called, but game mode is racing");
    }
    return pathfinding_waypoints_;
}

PlaybackWaypoints& Player::playback_waypoints() {
    if (game_mode_ != GameMode::RACING) {
        throw std::runtime_error("Player::playback_waypoints called, but game mode is not racing");
    }
    return playback_waypoints_;
}
    
void Player::set_create_externals(const std::function<void(const std::string&, ExternalsMode, const std::unordered_map<ControlSource, Skills>&)>& create_externals)
{
    if (externals_mode_ != ExternalsMode::NONE) {
        throw std::runtime_error("Externals already created (1)");
    }
    if (vehicle_.create_externals) {
        throw std::runtime_error("create_externals already set");
    }
    vehicle_.create_externals = create_externals;
}

void Player::append_delete_externals(
    SceneNode* node,
    const std::function<void()>& delete_externals)
{
    // Consider reading the line
    // "delete_externals_.erase((SceneNode*)destroyed_object);"
    // in "Player::notify_destroyed" and the comments above it.
    delete_externals_.insert({ node, delete_externals });
    if (node != nullptr) {
        node->add_destruction_observer(this, true);
    }
}

void Player::notify_lap_time(
    float lap_time,
    const std::list<TrackElement>& track)
{
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    players_.notify_lap_time(this, lap_time, track);
}

void Player::notify_vehicle_destroyed() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    reset_node();
}
