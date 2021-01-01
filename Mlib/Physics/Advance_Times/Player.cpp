#include "Player.hpp"
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Containers/Collision_Query.hpp>
#include <Mlib/Physics/Containers/Players.hpp>
#include <Mlib/Physics/Interfaces/Damageable.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Scene.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

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
    std::recursive_mutex& mutex)
: scene_{scene},
  collision_query_{collision_query},
  players_{players},
  name_{name},
  team_{team},
  scene_node_{nullptr},
  target_scene_node_{nullptr},
  rb_{nullptr},
  target_rbi_{nullptr},
  ypln_{nullptr},
  gun_{nullptr},
  surface_power_forward_{NAN},
  surface_power_backward_{NAN},
  waypoint_{fixed_nans<float, 2>()},
  waypoint_id_{SIZE_MAX},
  waypoint_reached_{false},
  nwaypoints_reached_{0},
  game_mode_{game_mode},
  unstuck_mode_{unstuck_mode},
  driving_mode_{driving_mode},
  driving_direction_{driving_direction},
  mutex_{mutex},
  spotted_{false}
{}

void Player::set_rigid_body(const std::string& scene_node_name, SceneNode& scene_node, RigidBody& rb) {
    if (scene_node_ != nullptr || rb_ != nullptr) {
        throw std::runtime_error("Player rb already set");
    }
    if (scene_node_name.empty()) {
        throw std::runtime_error("Player received empty node name");
    }
    scene_node_name_ = scene_node_name;
    scene_node_ = &scene_node;
    if (rb_ != nullptr) {
        if (rb_->driver_ != this) {
            throw std::runtime_error("Old rigid body has unexpected driver");
        }
        rb_->driver_ = nullptr;
    }
    rb_ = &rb;
    rb.driver_ = this;
    scene_node.add_destruction_observer(this);
}

const std::string& Player::scene_node_name() const {
    return scene_node_name_;
}

void Player::set_ypln(YawPitchLookAtNodes& ypln, Gun* gun) {
    if (ypln_ != nullptr || gun_ != nullptr) {
        throw std::runtime_error("ypln already set");
    }
    ypln_ = &ypln;
    gun_ = gun;
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

void Player::set_tire_angle_y(size_t tire_id, float angle_left, float angle_right) {
    {
        auto it = tire_angles_left_.insert(std::make_pair(tire_id, angle_left));
        if (!it.second) {
            throw std::runtime_error("tire angle left already set");
        }
    }
    {
        auto it = tire_angles_right_.insert(std::make_pair(tire_id, angle_right));
        if (!it.second) {
            throw std::runtime_error("tire angle right already set");
        }
    }
}

void Player::set_waypoint(const FixedArray<float, 2>& waypoint) {
    waypoint_ = waypoint;
    waypoint_id_ = SIZE_MAX;
    waypoint_reached_ = false;
}

void Player::set_waypoint(size_t waypoint_id) {
    waypoint_ = waypoints_.points.at(waypoint_id);
    waypoint_id_ = waypoint_id;
    waypoint_reached_ = false;
}

void Player::set_waypoints(const SceneNode& node, const PointsAndAdjacency<float, 2>& waypoints) {
    waypoints_ = waypoints;
    waypoints_.adjacency = waypoints_.adjacency / node.scale();
    FixedArray<float, 4, 4> m = node.absolute_model_matrix();
    FixedArray<float, 2, 3> m2{
        m(0, 0), m(0, 1), m(0, 3),
        m(2, 0), m(2, 1), m(2, 3)};
    for (FixedArray<float, 2>& p : waypoints_.points) {
        p = dot1d(m2, homogenized_3(p));
    }
    last_visited_ = std::vector<std::chrono::time_point<std::chrono::steady_clock>>(waypoints_.points.size());
    waypoint_id_ = SIZE_MAX;
    nwaypoints_reached_ = 0;
}

const std::string& Player::name() const {
    return name_;
}

const std::string& Player::team() const {
    return team_;
}

PlayerStats& Player::stats() {
    return stats_;
}

const PlayerStats& Player::stats() const {
    return stats_;
}

float Player::car_health() const {
    if (rb_ != nullptr && rb_->damageable_ != nullptr) {
        return rb_->damageable_->health();
    } else {
        return NAN;
    }
}

GameMode Player::game_mode() const {
    return game_mode_;
}

bool Player::can_see(const RigidBodyIntegrator& rbi, bool only_terrain) const {
    if (rb_ == nullptr) {
        throw std::runtime_error("Player::can_see requires rb");
    }
    return collision_query_.can_see(rb_->rbi_, rbi, only_terrain);
}

bool Player::can_see(const FixedArray<float, 3>& pos, float height_offset, float time_offset, bool only_terrain) const {
    if (rb_ == nullptr) {
        throw std::runtime_error("Player::can_see requires rb");
    }
    FixedArray<float, 3> d = {0, height_offset, 0};
    if (time_offset != 0) {
        RigidBodyPulses rbp = rb_->rbi_.rbp_;
        rbp.advance_time(time_offset);
        return collision_query_.can_see(rbp.abs_position() + d, pos + d, &rb_->rbi_, nullptr, only_terrain);
    } else {
        return collision_query_.can_see(rb_->rbi_.abs_position() + d, pos + d, &rb_->rbi_, nullptr, only_terrain);
    }
}

bool Player::can_see(const Player& player, bool only_terrain) const {
    if (rb_ == nullptr) {
        throw std::runtime_error("Player::can_see requires rb");
    }
    if (player.rb_ == nullptr) {
        throw std::runtime_error("Player::can_see requires rb");
    }
    return collision_query_.can_see(rb_->rbi_, player.rb_->rbi_, only_terrain);
}

void Player::notify_spawn() {
    for (auto& l : last_visited_) {
        l = std::chrono::time_point<std::chrono::steady_clock>();
    }
    set_waypoint(fixed_nans<float, 2>());
    nwaypoints_reached_ = 0;
    spawn_time_ = std::chrono::steady_clock::now();
    spotted_ = false;
}

float Player::seconds_since_spawn() const {
    if (spawn_time_ == std::chrono::time_point<std::chrono::steady_clock>()) {
        throw std::runtime_error("Seconds since spawn requires previous call to notify_spawn");
    }
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - spawn_time_).count() / 1000.f;
}

bool Player::spotted() const {
    return spotted_;
}
void Player::set_spotted() {
    spotted_ = true;
}

bool Player::has_waypoints() const {
    return !waypoints_.points.empty();
}

void Player::notify_destroyed(void* destroyed_object) {
    if (destroyed_object == scene_node_) {
        scene_node_name_.clear();
        scene_node_ = nullptr;
        rb_ = nullptr;
        ypln_ = nullptr;
    }
    if (destroyed_object == target_scene_node_) {
        target_scene_node_ = nullptr;
        target_rbi_ = nullptr;
    }
}

void Player::advance_time(float dt) {
    aim_and_shoot();
}

void Player::increment_external_forces(const std::list<std::shared_ptr<RigidBody>>& olist, bool burn_in, const PhysicsEngineConfig& cfg) {
    if (!burn_in) {
        if ((unstuck_mode_ == UnstuckMode::OFF) || !unstuck()) {
            if (ramming()) {
                auto tpos = target_rbi_->abs_position();
                set_waypoint({tpos(0), tpos(2)});
            } else {
                select_next_waypoint();
            }
            move_to_waypoint();
        }
    }
}

bool Player::unstuck() {
    if (rb_ == nullptr) {
        return false;
    }
    if ((sum(squared(rb_->rbi_.rbp_.v_)) > squared(driving_mode_.stuck_velocity)) ||
        (unstuck_start_ != std::chrono::steady_clock::time_point()))
    {
        stuck_start_ = std::chrono::steady_clock::now();
    } else if (
        (stuck_start_ != std::chrono::steady_clock::time_point()) &&
        (unstuck_start_ == std::chrono::steady_clock::time_point()) &&
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - stuck_start_).count() > driving_mode_.stuck_seconds * 1000)
    {
        unstuck_start_ = std::chrono::steady_clock::now();
    }
    if (unstuck_start_ != std::chrono::steady_clock::time_point()) {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - unstuck_start_).count() > driving_mode_.unstuck_seconds * 1000)
        {
            unstuck_start_ = std::chrono::steady_clock::time_point();
        } else {
            if (unstuck_mode_ == UnstuckMode::REVERSE) {
                rb_->set_surface_power("main", surface_power_backward_);
                for (auto &tire : rb_->tires_) {
                    rb_->set_tire_angle_y(tire.first, 0);
                }
            } else if (unstuck_mode_ == UnstuckMode::DELETE) {
                std::lock_guard lock{mutex_};
                scene_.delete_root_node(scene_node_name_);
                stuck_start_ = std::chrono::steady_clock::time_point();
                unstuck_start_ = std::chrono::steady_clock::time_point();
            } else {
                throw std::runtime_error("Unsupported unstuck mode");
            }
            return true;
        }
    }
    return false;
}

void Player::aim_and_shoot() {
    assert_true(!target_scene_node_ == !target_rbi_);
    if (rb_ != nullptr && ((target_rbi_ == nullptr) || !can_see(*target_rbi_))) {
        select_opponent();
    }
    if (ypln_ == nullptr) {
        return;
    }
    assert_true(scene_node_ == nullptr || scene_node_ != target_scene_node_);
    ypln_->set_followed(target_scene_node_, target_rbi_);
    if (gun_ == nullptr) {
        return;
    }
    if (target_scene_node_ != nullptr) {
        gun_->trigger();
    }
}

void Player::select_next_waypoint() {
    if (!waypoints_.adjacency.initialized()) {
        return;
    }
    if (rb_ == nullptr) {
        return;
    }
    FixedArray<float, 2> z2{rb_->rbi_.abs_z()(0), rb_->rbi_.abs_z()(2)};
    FixedArray<float, 2> pos2{rb_->rbi_.abs_position()(0), rb_->rbi_.abs_position()(2)};
    if (waypoint_id_ == SIZE_MAX) {
        // If we have no current waypoint, find closest point in waypoints array.
        size_t closest_id = SIZE_MAX;
        float closest_distance2 = INFINITY;
        size_t i = 0;
        for (const FixedArray<float, 2>& rs : waypoints_.points) {
            if (dot0d(rs - pos2, z2) < 0) {
                float dist2 = sum(squared(rs - pos2));
                if (dist2 < closest_distance2) {
                    closest_distance2 = dist2;
                    closest_id = i;
                }
            }
            ++i;
        }
        if (closest_id != SIZE_MAX) {
            set_waypoint(closest_id);
        }
    } else {
        if (waypoint_reached_) {
            if (nwaypoints_reached_ < 2) {
                // If we already have less than two waypoints, go further forward.
                size_t best_id = SIZE_MAX;
                float best_distance = INFINITY;
                for (const auto& rs : waypoints_.adjacency.column(waypoint_id_)) {
                    float dist = dot0d(waypoints_.points.at(rs.first) - pos2, z2);
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
                size_t best_id = SIZE_MAX;
                std::chrono::time_point<std::chrono::steady_clock> best_time;
                auto deflt = std::chrono::time_point<std::chrono::steady_clock>();
                for (const auto& rs : waypoints_.adjacency.column(waypoint_id_)) {
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
    return (game_mode_ == GameMode::RAMMING) && (target_rbi_ != nullptr);
}

void Player::move_to_waypoint() {
    if (rb_ == nullptr) {
        return;
    }
    if (std::isnan(surface_power_forward_)) {
        return;
    }
    if (std::isnan(surface_power_backward_)) {
        return;
    }
    auto step_on_breaks = [this](){
        rb_->set_surface_power("main", NAN);    // NAN=break
        rb_->set_surface_power("breaks", NAN);  // NAN=break
    };
    if (any(isnan(waypoint_))) {
        step_on_breaks();
        return;
    }
    // Stop when distance to waypoint is small enough (break).
    if (!ramming()) {
        FixedArray<float, 2> pos2{rb_->rbi_.abs_position()(0), rb_->rbi_.abs_position()(2)};
        if (sum(squared(pos2 - waypoint_)) < squared(driving_mode_.rest_radius)) {
            step_on_breaks();
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
    for (const auto& p : players_.players()) {
        if (p.second == this) {
            continue;
        }
        if (p.second->rb_ == nullptr) {
            continue;
        }
        if (ramming() &&
            (&p.second->rb_->rbi_ == target_rbi_))
        {
            continue;
        }
        FixedArray<float, 3> d = p.second->rb_->rbi_.abs_position() - rb_->rbi_.abs_position();
        float dl2 = sum(squared(d));
        if (dl2 < squared(driving_mode_.collision_avoidance_radius_break)) {
            auto z = rb_->rbi_.abs_z();
            if (dot0d(d, z) < 0) {
                step_on_breaks();
                return;
            }
        } else if (dl2 < squared(driving_mode_.collision_avoidance_radius_correct)) {
            if (dl2 > 1e-12) {
                auto z = rb_->rbi_.abs_z();
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
        float dvel = sum(squared(rb_->rbi_.rbp_.v_)) - squared(driving_mode_.max_velocity);
        if (dvel < 0) {
            rb_->set_surface_power("main", surface_power_forward_);
        } else if (dvel < squared(driving_mode_.max_delta_velocity2_break)) {
            rb_->set_surface_power("main", 0);
        } else {
            step_on_breaks();
        }
    }
    // Steer towards waypoint.
    // FixedArray<float, 3> wp{waypoint_(0), 0, waypoint_(1)};
    // auto m = rb_->get_new_absolute_model_matrix();
    // auto v = inverted_scaled_se3(m);
    // auto wpt = dehomogenized_3(dot1d(v, homogenized_4(wp)));
    auto z3 = rb_->rbi_.rbp_.abs_z();
    FixedArray<float, 2> z{z3(0), z3(2)};
    float zl2 = sum(squared(z));
    if (zl2 > 1e-12) {
        z /= std::sqrt(zl2);
        auto p = rb_->rbi_.rbp_.abs_position();
        auto wpt = waypoint_ - FixedArray<float, 2>(p(0), p(2));
        FixedArray<float, 2, 2> m{
            z(1), -z(0),
            z(0), z(1)};
        wpt = dot1d(m, wpt);
        if (sum(squared(wpt)) > 1e-12) {
            wpt += FixedArray<float, 2>(-wpt(1), wpt(0)) * d_wpt;
            if (wpt(1) > 0) {
                // The waypoint is behind us => full, inverted steering.
                if (wpt(0) < 0) {
                    for (const auto& x : tire_angles_left_) {
                        rb_->set_tire_angle_y(x.first, x.second);
                    }
                } else {
                    for (const auto& x : tire_angles_right_) {
                        rb_->set_tire_angle_y(x.first, x.second);
                    }
                }
            } else {
                // The waypoint is in front of us => full, inverted steering.
                float angle = std::atan(std::abs(wpt(0) / wpt(1)));
                if (wpt(0) < 0) {
                    for (const auto& x : tire_angles_left_) {
                        float ang = sign(x.second) * std::min(angle, std::abs(x.second));
                        rb_->set_tire_angle_y(x.first, ang);
                    }
                } else {
                    for (const auto& x : tire_angles_right_) {
                        float ang = sign(x.second) * std::min(angle, std::abs(x.second));
                        rb_->set_tire_angle_y(x.first, ang);
                    }
                }
            }
        }
    }
}

void Player::select_opponent() {
    assert_true(!scene_node_ == !rb_);
    if (rb_ == nullptr) {
        return;
    }
    if (target_scene_node_ != nullptr) {
        target_scene_node_->remove_destruction_observer(this);
        target_scene_node_ = nullptr;
        target_rbi_ = nullptr;
    }
    for (const auto& p : players_.players()) {
        if (p.second->team_ != team_) {
            assert_true(!p.second->scene_node_ == !p.second->rb_);
            if (p.second->rb_ == nullptr) {
                continue;
            }
            if (can_see(p.second->rb_->rbi_)) {
                target_scene_node_ = p.second->scene_node_;
                target_rbi_ = &p.second->rb_->rbi_;
                target_scene_node_->add_destruction_observer(this);
                break;
            }
        }
    }
}
