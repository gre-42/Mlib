#include "Player.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Advance_Times/Movables/Yaw_Pitch_Look_At_Nodes.hpp>
#include <Mlib/Physics/Containers/Collision_Query.hpp>
#include <Mlib/Physics/Containers/Players.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>

static const float rest_radius = 30;
static const float max_velocity = 10;
static const float max_velocity_break = 2;
static const float collision_avoidance_radius = 20;

using namespace Mlib;

Player::Player(
    CollisionQuery& collision_query,
    Players& players,
    const std::string& name,
    const std::string& team)
: collision_query_{collision_query},
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
  waypoint_reached_{false}
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
    rb_ = &rb;
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
    for(FixedArray<float, 2>& p : waypoints_.points) {
        p = dot1d(m2, homogenized_3(p));
    }
    last_visited_ = std::vector<std::chrono::time_point<std::chrono::steady_clock>>(waypoints_.points.size());
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
        select_next_waypoint();
        move_to_waypoint();
    }
}

void Player::aim_and_shoot() {
    assert_true(!target_scene_node_ == !target_rbi_);
    if (rb_ != nullptr && ((target_rbi_ == nullptr) || !collision_query_.can_see(rb_->rbi_, *target_rbi_))) {
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
    std::cerr << "a" << std::endl;
    if (!waypoints_.adjacency.initialized()) {
        return;
    }
    std::cerr << "a1" << std::endl;
    if (rb_ == nullptr) {
        return;
    }
    std::cerr << "a2" << std::endl;
    FixedArray<float, 2> pos2{rb_->rbi_.abs_position()(0), rb_->rbi_.abs_position()(2)};
    std::cerr << "a3" << std::endl;
    if (waypoint_id_ == SIZE_MAX) {
        std::cerr << "a4" << std::endl;
        size_t closest_id = SIZE_MAX;
        float closest_distance2 = INFINITY;
        size_t i = 0;
        std::cerr << "a5" << std::endl;
        for(const FixedArray<float, 2>& rs : waypoints_.points) {
            std::cerr << "a6" << std::endl;
            float dist2 = sum(squared(rs - pos2));
            if (dist2 < closest_distance2) {
                closest_distance2 = dist2;
                closest_id = i;
            }
            ++i;
        }
        std::cerr << "a7" << std::endl;
        if (closest_id != SIZE_MAX) {
            std::cerr << "ax" << std::endl;
            set_waypoint(closest_id);
        }
        std::cerr << "a8" << std::endl;
    } else {
        std::cerr << "1a9" << std::endl;
        if (waypoint_reached_) {
            std::cerr << "2a9" << std::endl;
            size_t best_id = SIZE_MAX;
            std::chrono::time_point<std::chrono::steady_clock> best_time;
            std::cerr << "3a9" << std::endl;
            auto deflt = std::chrono::time_point<std::chrono::steady_clock>();
            for(const auto& rs : waypoints_.adjacency.column(waypoint_id_)) {
                if ((best_id == SIZE_MAX) ||
                    (last_visited_.at(rs.first) == deflt) ||
                    ((best_time != deflt) && (last_visited_.at(rs.first) > best_time)))
                {
                    best_id = rs.first;
                    best_time = last_visited_.at(rs.first);
                    std::cerr << "4a9 " << best_id << " " << best_time.time_since_epoch().count() << std::endl;
                }
            }
            std::cerr << "5a9" << std::endl;
            if (best_id == SIZE_MAX) {
                throw std::runtime_error("Select next waypoint failed. Forgot diagonal elements of adjacency matrix?");
            }
            set_waypoint(best_id);
        }
        std::cerr << "a10" << std::endl;
    }
}

void Player::move_to_waypoint() {
    if (any(isnan(waypoint_))) {
        return;
    }
    if (rb_ == nullptr) {
        return;
    }
    if (std::isnan(surface_power_forward_)) {
        return;
    }
    if (std::isnan(surface_power_backward_)) {
        return;
    }
    // Stop when distance to waypoint is small enough (break).
    {
        FixedArray<float, 2> pos2{rb_->rbi_.abs_position()(0), rb_->rbi_.abs_position()(2)};
        if (sum(squared(pos2 - waypoint_)) < squared(rest_radius)) {
            rb_->set_surface_power("main", NAN);  // NAN=break
            if (waypoint_id_ != SIZE_MAX) {
                last_visited_.at(waypoint_id_) = std::chrono::steady_clock::now();
            }
            waypoint_reached_ = true;
            return;
        }
    }
    // Avoid collisions with other players (break).
    for (const auto& p : players_.players()) {
        if (p.second == this) {
            continue;
        }
        if (p.second->rb_ == nullptr) {
            continue;
        }
        FixedArray<float, 3> d = p.second->rb_->rbi_.abs_position() - rb_->rbi_.abs_position();
        if (sum(squared(d)) < squared(collision_avoidance_radius)) {
            auto z = rb_->rbi_.abs_z();
            if (dot0d(d, z) < 0) {
                rb_->set_surface_power("main", NAN);  // NAN=break
                return;
            }
        }
    }
    // Keep velocity within the specified range.
    {
        float dvel = sum(squared(rb_->rbi_.rbp_.v_)) - squared(max_velocity);
        if (dvel < 0) {
            rb_->set_surface_power("main", surface_power_forward_);
        } else if (dvel < squared(max_velocity_break)) {
            rb_->set_surface_power("main", 0);
        } else {
            rb_->set_surface_power("main", NAN);  // NAN=break
        }
    }
    // Steer towards waypoint.
    FixedArray<float, 3> wp{waypoint_(0), 0, waypoint_(1)};
    auto m = rb_->get_new_absolute_model_matrix();
    auto v = inverted_scaled_se3(m);
    auto wpt = dot1d(v, homogenized_4(wp));
    if (wpt(2) > 0) {
        if (wpt(0) < 0) {
            for(const auto& x : tire_angles_left_) {
                rb_->set_tire_angle_y(x.first, x.second);
            }
        } else {
            for(const auto& x : tire_angles_right_) {
                rb_->set_tire_angle_y(x.first, x.second);
            }
        }
    } else {
        float angle = std::atan(std::abs(wpt(0) / wpt(2))) * 180 / M_PI;
        if (wpt(0) < 0) {
            for(const auto& x : tire_angles_left_) {
                float ang = sign(x.second) * std::min(angle, std::abs(x.second));
                rb_->set_tire_angle_y(x.first, ang);
            }
        } else {
            for(const auto& x : tire_angles_right_) {
                float ang = sign(x.second) * std::min(angle, std::abs(x.second));
                rb_->set_tire_angle_y(x.first, ang);
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
            if (collision_query_.can_see(rb_->rbi_, p.second->rb_->rbi_)) {
                target_scene_node_ = p.second->scene_node_;
                target_rbi_ = &p.second->rb_->rbi_;
                target_scene_node_->add_destruction_observer(this);
                break;
            }
        }
    }
}
