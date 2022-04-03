#include "Player.hpp"
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Pitch_Look_At_Node.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Containers/Collision_Query.hpp>
#include <Mlib/Physics/Interfaces/Damageable.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Mlib_Pod_Bot/Pod_Bot_Player.hpp>
#include <Mlib/Players/Pod_Bot_Mlib_Compat/mlib.hpp>
#include <Mlib/Recursive_Deletion.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Style_Updater.hpp>
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
      .rb{ nullptr },
      .ypln{ nullptr },
      .gun_node{ nullptr }
  },
  target_scene_node_{ nullptr },
  target_rb_{ nullptr },
  surface_power_forward_{ NAN },
  surface_power_backward_{ NAN },
  waypoint_{ fixed_nans <float, 3>()},
  waypoint_id_{ SIZE_MAX },
  waypoint_reached_{ false },
  nwaypoints_reached_{ 0 },
  game_mode_{ game_mode },
  unstuck_mode_{ unstuck_mode },
  driving_mode_{ driving_mode },
  driving_direction_{ driving_direction },
  spotted_by_vip_{ false },
  nunstucked_{ 0 },
  record_waypoints_{ false },
  delete_node_mutex_{ delete_node_mutex },
  next_scene_node_{ nullptr },
  externals_created_{ false }
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

void Player::set_can_drive(bool value) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    skills_.can_drive = value;
}

void Player::set_can_aim(bool value) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    skills_.can_aim = value;
}

void Player::set_can_shoot(bool value) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    skills_.can_shoot = value;
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
    vehicle_.gun_node = nullptr;
    if (next_scene_node_ != nullptr) {
        next_scene_node_->remove_destruction_observer(this);
        next_scene_node_ = nullptr;
    }
    if (target_scene_node_ != nullptr) {
        target_scene_node_->remove_destruction_observer(this);
        target_scene_node_ = nullptr;
        target_rb_ = nullptr;
        if (vehicle_.ypln != nullptr) {
            vehicle_.ypln->set_followed(nullptr, nullptr);
        }
    }
    vehicle_.ypln = nullptr;
    surface_power_forward_ = NAN;
    surface_power_backward_ = NAN;
    stuck_start_ = std::chrono::steady_clock::time_point();
    unstuck_start_ = std::chrono::steady_clock::time_point();
    if (!delete_externals_.empty()) {
        std::lock_guard lock{ delete_node_mutex_ };
        clear_map_recursively(delete_externals_, [](const auto& p){
            p.mapped()();
        });
        externals_created_ = false;
    }
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
    if (vehicle_.ypln != nullptr || vehicle_.gun_node != nullptr) {
        throw std::runtime_error("ypln already set");
    }
    vehicle_.ypln = &ypln;
    vehicle_.gun_node = gun_node;
}

void Player::set_surface_power(float forward, float backward) {
    if (!std::isnan(surface_power_forward_)) {
        throw std::runtime_error("surface_power_forward already set");
    }
    surface_power_forward_ = forward;
    if (!std::isnan(surface_power_backward_)) {
        throw std::runtime_error("surface_power_backward already set");
    }
    surface_power_backward_ = backward;
}

void Player::set_waypoint(const FixedArray<float, 3>& waypoint, size_t waypoint_id) {
    waypoint_ = waypoint;
    waypoint_id_ = waypoint_id;
    if (record_waypoints_ && !any(Mlib::isnan(waypoint))) {
        waypoint_history_.push_back(waypoint);
    }
    waypoint_reached_ = false;
}

void Player::set_waypoint(const FixedArray<float, 3>& waypoint) {
    set_waypoint(waypoint, SIZE_MAX);
}

void Player::set_waypoint(size_t waypoint_id) {
    set_waypoint(waypoints().points.at(waypoint_id), waypoint_id);
}

void Player::set_waypoints(
    const SceneNode& node,
    const std::map<WayPointLocation, PointsAndAdjacency<float, 3>>& all_waypoints)
{
    all_waypoints_bvh_.clear();
    all_waypoints_ = all_waypoints;
    TransformationMatrix<float, 3> m = node.absolute_model_matrix();
    for (auto& wps : all_waypoints_) {
        wps.second.adjacency = wps.second.adjacency * m.get_scale();
        Bvh<float, size_t, 3> bvh{{100.f, 100.f, 100.f}, 10};
        size_t i = 0;
        for (FixedArray<float, 3>& p : wps.second.points) {
            p = m.transform(p);
            bvh.insert(p, i++);
        }
        // bvh.optimize_search_time(std::cout);
        if (!all_waypoints_bvh_.insert({wps.first, std::move(bvh)}).second) {
            throw std::runtime_error("Could not insert waypoints");
        }
    }
    last_visited_ = std::vector<std::chrono::time_point<std::chrono::steady_clock>>(waypoints().points.size());
    waypoint_id_ = SIZE_MAX;
    nwaypoints_reached_ = 0;
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
    return has_rigid_body() ? vehicle_.rb->name() : "";
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
    const FixedArray<float, 3>& pos,
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
    for (auto& l : last_visited_) {
        l = std::chrono::time_point<std::chrono::steady_clock>();
    }
    set_waypoint(fixed_nans<float, 3>());
    nwaypoints_reached_ = 0;
    spawn_time_ = std::chrono::steady_clock::now();
    spotted_by_vip_ = false;
    waypoint_history_.clear();
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
        } else if ((unstuck_mode_ == UnstuckMode::OFF) || !unstuck()) {
            if (ramming()) {
                auto tpos = target_rb_->abs_target();
                set_waypoint(tpos);
            } else {
                if (has_waypoints()) {
                    select_next_waypoint();
                }
            }
            move_to_waypoint();
        }
    }
}

void Player::draw_waypoint_history(const std::string& filename) const {
    delete_node_mutex_.notify_reading();
    if (!record_waypoints_) {
        throw std::runtime_error("draw_waypoint_history but recording is not enabled");
    }
    std::ofstream ofstr{filename};
    if (ofstr.fail()) {
        throw std::runtime_error("Could not open \"" + filename + "\" for write");
    }
    Svg<float> svg{ofstr, 600, 600};
    std::vector<float> x;
    std::vector<float> y;
    x.resize(waypoint_history_.size());
    y.resize(waypoint_history_.size());
    size_t i = 0;
    for (const auto& w : waypoint_history_) {
        x[i] = w(0);
        y[i] = w(2);
        ++i;
    }
    svg.plot(x, y);
    svg.finish();
    ofstr.flush();
    if (ofstr.fail()) {
        throw std::runtime_error("Could not write to file \"" + filename + '"');
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
    if (vehicle_.gun_node == nullptr) {
        throw std::runtime_error("gun_direction despite gun nullptr in player \"" + name() + '"');
    }
    return -z3_from_3x3(gun()->absolute_model_matrix().R());
}

FixedArray<float, 3> Player::punch_angle() const {
    delete_node_mutex_.notify_reading();
    if (vehicle_.gun_node == nullptr) {
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
    if (vehicle_.ypln == nullptr) {
        throw std::runtime_error("run_move despite ypln nullptr");
    }

    vehicle_.ypln->set_yaw(yaw);
    vehicle_.ypln->pitch_look_at_node()->set_pitch(pitch);

    FixedArray<float, 3> direction{ sidemove, 0.f, -forwardmove };
    float len2 = sum(squared(direction));
    if (len2 < 1e-12) {
        step_on_brakes();
    } else {
        float len = std::sqrt(len2);
        vehicle_.rb->tires_z_ = direction / len;
        vehicle_.rb->set_surface_power("legs", len * surface_power_forward_);
    }
    if (vehicle_.rb->style_updater_ != nullptr) {
        vehicle_.rb->style_updater_->notify_movement_intent();
    }
}

void Player::trigger_gun() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (vehicle_.gun_node == nullptr) {
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

void Player::steer_left_full() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    vehicle_.rb->vehicle_controller().steer(INFINITY);
}

void Player::steer_right_full() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    vehicle_.rb->vehicle_controller().steer(-INFINITY);
}

void Player::steer_left_partial(float angle) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    vehicle_.rb->vehicle_controller().steer(angle);
}

void Player::steer_right_partial(float angle) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    vehicle_.rb->vehicle_controller().steer(-angle);
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

const PointsAndAdjacency<float, 3>& Player::waypoints() const {
    delete_node_mutex_.notify_reading();
    auto it = all_waypoints_.find(driving_mode_.way_point_location);
    if (it == all_waypoints_.end()) {
        throw std::runtime_error("Could not find waypoints for the specified location");
    }
    return it->second;
}

bool Player::has_waypoints() const {
    delete_node_mutex_.notify_reading();
    auto it = all_waypoints_.find(driving_mode_.way_point_location);
    if (it == all_waypoints_.end()) {
        return false;
    }
    return !it->second.points.empty();
}

const Gun* Player::gun() const {
    if (vehicle_.gun_node == nullptr) {
        throw std::runtime_error("Gun node not set");
    }
    Gun* gun = dynamic_cast<Gun*>(vehicle_.gun_node->get_absolute_observer());
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
    if (!skills_.can_aim) {
        return;
    }
    if (game_mode_ == GameMode::POD_BOT_NPC) {
        return;
    }
    assert_true(!target_scene_node_ == !target_rb_);
    if (has_rigid_body() && ((target_rb_ == nullptr) || !can_see(*target_rb_))) {
        select_next_opponent();
    }
    if (vehicle_.ypln == nullptr) {
        return;
    }
    assert_true(vehicle_.scene_node == nullptr || vehicle_.scene_node != target_scene_node_);
    vehicle_.ypln->set_followed(target_scene_node_, target_rb_);
    if (vehicle_.gun_node == nullptr) {
        return;
    }
    if (!skills_.can_shoot) {
        return;
    }
    if ((target_scene_node_ != nullptr) && (vehicle_.ypln->target_locked_on())) {
        gun()->trigger();
    }
}

void Player::select_next_waypoint() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!waypoints().adjacency.initialized()) {
        return;
    }
    if (!has_rigid_body()) {
        return;
    }
    FixedArray<float, 3> z3 = vehicle_.rb->rbi_.abs_z();
    FixedArray<float, 3> pos3 = vehicle_.rb->rbi_.abs_position();
    if (waypoint_id_ == SIZE_MAX) {
        // If we have no current waypoint, find closest point in waypoints array.
        float max_distance = 100;
        size_t closest_id = SIZE_MAX;
        float closest_distance2 = INFINITY;
        const auto& wps = waypoints();
        all_waypoints_bvh_.at(driving_mode_.way_point_location).visit(
            {pos3, max_distance},
            [&](size_t i)
        {
            const auto& rs = wps.points.at(i);
            auto diff = rs - pos3;
            auto dist2 = sum(squared(diff));
            if ((dist2 < 1e-6) ||
                (dot0d(diff / std::sqrt(dist2), z3) < -std::cos(45.f * degrees)))
            {
                if (dist2 < closest_distance2) {
                    closest_distance2 = dist2;
                    closest_id = i;
                }
            }
            return true;
        });
        if (closest_id != SIZE_MAX) {
            set_waypoint(closest_id);
        }
    } else {
        if (waypoint_reached_) {
            if (nwaypoints_reached_ < 2) {
                // If we already have less than two waypoints, go further forward.
                size_t best_id = SIZE_MAX;
                float best_distance = INFINITY;
                for (const auto& rs : waypoints().adjacency.column(waypoint_id_)) {
                    float dist = dot0d(waypoints().points.at(rs.first) - pos3, z3);
                    if (dist < best_distance) {
                        best_id = rs.first;
                        best_distance = dist;
                    }
                }
                if (best_id == SIZE_MAX) {
                    throw std::runtime_error("Select next waypoint failed. Forgot diagonal elements of adjacency matrix?");
                }
                set_waypoint(best_id);
            } else {
                // If we already have two waypoints, pick oldest neighbor.
                auto deflt = std::chrono::time_point<std::chrono::steady_clock>();
                size_t best_id = SIZE_MAX;
                auto best_time = deflt;
                for (const auto& rs : waypoints().adjacency.column(waypoint_id_)) {
                    if ((best_id == SIZE_MAX) ||
                        (last_visited_.at(rs.first) == deflt) ||
                        ((best_time != deflt) && (last_visited_.at(rs.first) < best_time)))
                    {
                        best_id = rs.first;
                        best_time = last_visited_.at(rs.first);
                    }
                }
                if (best_id == SIZE_MAX) {
                    throw std::runtime_error("Select next waypoint failed. Forgot diagonal elements of adjacency matrix?");
                }
                set_waypoint(best_id);
            }
        }
    }
}

bool Player::ramming() const {
    delete_node_mutex_.notify_reading();
    return (game_mode_ == GameMode::RAMMING) && (target_rb_ != nullptr);
}

void Player::move_to_waypoint() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!skills_.can_drive) {
        return;
    }
    if (!has_rigid_body()) {
        return;
    }
    vehicle_.rb->vehicle_controller().reset(
        0.f, // surface_power
        0.f  // steer_angle
    );
    if (std::isnan(surface_power_forward_) ||
        std::isnan(surface_power_backward_) ||
        any(Mlib::isnan(waypoint_)))
    {
        step_on_brakes();
        vehicle_.rb->vehicle_controller().apply();
        return;
    }
    // Stop when distance to waypoint is small enough (break).
    if (!ramming()) {
        FixedArray<float, 3> pos3 = vehicle_.rb->rbi_.abs_position();
        if (sum(squared(pos3 - waypoint_)) < squared(driving_mode_.rest_radius)) {
            step_on_brakes();
            vehicle_.rb->vehicle_controller().apply();
            if (waypoint_id_ != SIZE_MAX) {
                last_visited_.at(waypoint_id_) = std::chrono::steady_clock::now();
            }
            waypoint_reached_ = true;
            ++nwaypoints_reached_;
            return;
        }
    }
    float d_wpt = 0;
    // Avoid collisions with other players (break).
    for (const auto& [_, p] : players_.players()) {
        if (p.get() == this) {
            continue;
        }
        if (!p->has_rigid_body()) {
            continue;
        }
        if (ramming() &&
            (p->vehicle_.rb == target_rb_))
        {
            continue;
        }
        FixedArray<float, 3> d = p->vehicle_.rb->rbi_.abs_position() - vehicle_.rb->rbi_.abs_position();
        float dl2 = sum(squared(d));
        if (dl2 < squared(driving_mode_.collision_avoidance_radius_break)) {
            auto z = vehicle_.rb->rbi_.abs_z();
            if (dot0d(d, z) < 0) {
                step_on_brakes();
                vehicle_.rb->vehicle_controller().apply();
                return;
            }
        } else if (dl2 < squared(driving_mode_.collision_avoidance_radius_correct)) {
            if (dl2 > 1e-12) {
                auto z = vehicle_.rb->rbi_.abs_z();
                if (dot0d(d, z) / std::sqrt(dl2) < -driving_mode_.collision_avoidance_cos) {
                    if (driving_direction_ == DrivingDirection::CENTER || driving_direction_ == DrivingDirection::RIGHT) {
                        d_wpt = driving_mode_.collision_avoidance_delta;
                    } else if (driving_direction_ == DrivingDirection::LEFT) {
                        d_wpt = -driving_mode_.collision_avoidance_delta;
                    } else {
                        throw std::runtime_error("Unknown driving direction");
                    }
                }
            }
        }
    }
    // Keep velocity within the specified range.
    {
        float dvel = -dot0d(vehicle_.rb->rbi_.rbp_.v_, vehicle_.rb->rbi_.abs_z()) - driving_mode_.max_velocity;
        if (dvel < 0) {
            drive_forward();
        } else if (dvel < driving_mode_.max_delta_velocity_break) {
            roll_tires();
        } else {
            step_on_brakes();
        }
    }
    // Steer towards waypoint.
    // FixedArray<float, 3> wp{waypoint_(0), 0, waypoint_(1)};
    // auto m = vehicle_.rb->get_new_absolute_model_matrix();
    // auto v = inverted_scaled_se3(m);
    // auto wpt = dehomogenized_3(dot1d(v, homogenized_4(wp)));
    auto z3 = vehicle_.rb->rbi_.rbp_.abs_z();
    FixedArray<float, 2> z{z3(0), z3(2)};
    float zl2 = sum(squared(z));
    if (zl2 > 1e-12) {
        z /= std::sqrt(zl2);
        auto p = vehicle_.rb->rbi_.rbp_.abs_position();
        auto wpt = FixedArray<float, 2>{waypoint_(0), waypoint_(2)} - FixedArray<float, 2>{p(0), p(2)};
        FixedArray<float, 2, 2> m{
            z(1), -z(0),
            z(0), z(1)};
        wpt = dot1d(m, wpt);
        if (sum(squared(wpt)) > 1e-12) {
            wpt += FixedArray<float, 2>(-wpt(1), wpt(0)) * d_wpt;
            if (wpt(1) > 0) {
                // The waypoint is behind us => full, inverted steering.
                if (wpt(0) < 0) {
                    steer_left_full();
                } else {
                    steer_right_full();
                }
            } else {
                // The waypoint is in front of us => partial, inverted steering.
                float angle = std::atan(std::abs(wpt(0) / wpt(1)));
                if (wpt(0) < 0) {
                    steer_left_partial(angle);
                } else {
                    steer_right_partial(angle);
                }
            }
        }
    }
    vehicle_.rb->vehicle_controller().apply();
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

void Player::create_externals() {
    if (externals_created_) {
        throw std::runtime_error("Externals already created (0)");
    }
    if (!vehicle_.create_externals) {
        throw std::runtime_error("create_externals not set");
    }
    vehicle_.create_externals(name());
    externals_created_ = true;
}

bool Player::externals_created() const {
    return externals_created_;
}

void Player::set_create_externals(const std::function<void(const std::string&)>& create_externals)
{
    if (externals_created_) {
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
