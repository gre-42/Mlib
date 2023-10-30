#include "Single_Waypoint.hpp"
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Avatar_Controllers/Rigid_Body_Avatar_Controller.hpp>
#include <Mlib/Physics/Vehicle_Controllers/Car_Controllers/Rigid_Body_Vehicle_Controller.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Scene_Vehicle/Control_Source.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

// namespace Mlib { thread_local extern std::list<Beacon> g_beacons; }

SingleWaypoint::SingleWaypoint(Player& player)
: player_{player},
  target_velocity_{NAN},
  waypoint_{ fixed_nans <double, 3>()},
  waypoint_id_{ SIZE_MAX },
  previous_waypoint_id_{ SIZE_MAX },
  waypoint_reached_{ false },
  nwaypoints_reached_{ 0 },
  record_waypoints_{ false }
{}

SingleWaypoint::~SingleWaypoint()
{}

void SingleWaypoint::set_target_velocity(float v) {
    target_velocity_ = v;
}

void SingleWaypoint::set_waypoint(const FixedArray<double, 3>& waypoint, size_t waypoint_id) {
    previous_waypoint_id_ = waypoint_id_;
    waypoint_ = waypoint;
    waypoint_(1) += player_.driving_mode_.waypoint_ofs;
    waypoint_id_ = waypoint_id;
    if (record_waypoints_ && !any(Mlib::isnan(waypoint))) {
        waypoint_history_.push_back(waypoint);
    }
    waypoint_reached_ = false;
}

void SingleWaypoint::set_waypoint(const FixedArray<double, 3>& waypoint) {
    set_waypoint(waypoint, SIZE_MAX);
}

void SingleWaypoint::move_to_waypoint() {
    player_.delete_node_mutex_.assert_this_thread_is_deleter_thread();
    // if (!any(Mlib::isnan(waypoint_))) {
    //     g_beacons.push_back(Beacon::create(waypoint_, "flag"));
    // }
    if (!player_.skills_.at(ControlSource::AI).can_drive) {
        return;
    }
    if (!player_.has_scene_vehicle()) {
        return;
    }
    auto& player_rb = player_.rigid_body();
    // Disabled, using "steer" instead to enable the PID-controller.
    // player_rb.vehicle_controller().reset_parameters(
    //     0.f, // surface_power
    //     0.f  // steer_angle
    // );
    player_rb.vehicle_controller().reset_relaxation(0.f, 0.f);
    if (std::isnan(player_.vehicle_movement.surface_power_forward()) ||
        std::isnan(player_.vehicle_movement.surface_power_backward()) ||
        any(Mlib::isnan(waypoint_)))
    {
        player_.car_movement.step_on_brakes();
        player_.car_movement.steer(0.f);
        player_rb.vehicle_controller().apply();
        return;
    }
    // Stop when distance to waypoint is small enough (break).
    if (!player_.ramming()) {
        FixedArray<double, 3> pos3 = player_rb.rbi_.abs_position();
        double distance_to_waypoint2 = sum(squared(pos3 - waypoint_));
        float lookahead_fac2 = std::max(
            1.f,
            sum(squared(player_rb.rbi_.rbp_.v_)) /
            squared(player_.driving_mode_.lookahead_velocity));
        if (distance_to_waypoint2 < squared(player_.driving_mode_.waypoint_reached_radius) * lookahead_fac2) {
            if (waypoint_id_ != SIZE_MAX) {
                last_visited_.at(waypoint_id_) = std::chrono::steady_clock::now();
            }
            waypoint_reached_ = true;
            ++nwaypoints_reached_;
        }
        if (distance_to_waypoint2 < squared(player_.driving_mode_.rest_radius) * lookahead_fac2) {
            player_.car_movement.step_on_brakes();
            player_.car_movement.steer(0.f);
            player_rb.vehicle_controller().apply();
            return;
        }
    }
    float d_wpt = 0;
    // Avoid collisions with other players (brake).
    for (const auto& [_, p] : player_.players_.players()) {
        if (p.get() == &player_) {
            continue;
        }
        if (!p->has_scene_vehicle()) {
            continue;
        }
        auto& p_rb = p->rigid_body();
        if (player_.ramming() &&
            (&p_rb == player_.target_rb_))
        {
            continue;
        }
        FixedArray<double, 3> d = p_rb.rbi_.abs_position() - player_rb.rbi_.abs_position();
        double dl2 = sum(squared(d));
        if (dl2 < squared(player_.driving_mode_.collision_avoidance_radius_brake)) {
            auto z = player_rb.rbi_.abs_z();
            if (dot0d(d, z.casted<double>()) < 0) {
                player_.car_movement.step_on_brakes();
                player_.car_movement.steer(0.f);
                player_rb.vehicle_controller().apply();
                return;
            }
        } else if (dl2 < squared(player_.driving_mode_.collision_avoidance_radius_correct)) {
            if (dl2 > 1e-12) {
                auto z = player_rb.rbi_.abs_z();
                if (dot0d(d, z.casted<double>()) / std::sqrt(dl2) < -player_.driving_mode_.collision_avoidance_cos) {
                    if (player_.driving_direction_ == DrivingDirection::CENTER || player_.driving_direction_ == DrivingDirection::RIGHT) {
                        d_wpt = player_.driving_mode_.collision_avoidance_delta;
                    } else if (player_.driving_direction_ == DrivingDirection::LEFT) {
                        d_wpt = -player_.driving_mode_.collision_avoidance_delta;
                    } else {
                        THROW_OR_ABORT("Unknown driving direction");
                    }
                }
            }
        }
    }
    if (player_rb.avatar_controller_ != nullptr) {
        player_rb.avatar_controller_->reset();
    }
    // Keep velocity within the specified range.
    {
        float target_vel = std::isnan(target_velocity_)
            ? player_.driving_mode_.max_velocity
            : target_velocity_;
        float dvel = -dot0d(player_rb.rbi_.rbp_.v_, player_rb.rbi_.abs_z()) - target_vel;
        if (dvel < 0) {
            if (player_rb.avatar_controller_ != nullptr) {
                player_rb.avatar_controller_->walk(player_.vehicle_movement.surface_power_forward());
            } else {
                player_.car_movement.drive_forward();
            }
        } else if (dvel < player_.driving_mode_.max_delta_velocity_brake) {
            if (player_rb.avatar_controller_ != nullptr) {
                player_rb.avatar_controller_->walk(0.f);
            } else {
                player_.car_movement.roll_tires();
            }
        } else {
            if (player_rb.avatar_controller_ != nullptr) {
                player_rb.avatar_controller_->stop();
            } else {
                player_.car_movement.step_on_brakes();
            }
        }
    }
    // Steer towards waypoint.
    // FixedArray<float, 3> wp{waypoint_(0), 0, waypoint_(1)};
    // auto m = vehicle_.rb->get_new_absolute_model_matrix();
    // auto v = inverted_scaled_se3(m);
    // auto wpt = dehomogenized_3(dot1d(v, homogenized_4(wp)));
    auto z3 = player_rb.rbi_.rbp_.abs_z();
    FixedArray<float, 2> z{z3(0), z3(2)};
    float zl2 = sum(squared(z));
    if (zl2 > 1e-12) {
        z /= std::sqrt(zl2);
        auto p = player_rb.rbi_.rbp_.abs_position();
        auto wpt = FixedArray<double, 2>{waypoint_(0), waypoint_(2)} - FixedArray<double, 2>{p(0), p(2)};
        auto m = FixedArray<double, 2, 2>::init(
            z(1), -z(0),
            z(0), z(1));
        wpt = dot1d(m, wpt);
        wpt += FixedArray<double, 2>(-wpt(1), wpt(0)) * double(d_wpt);
        double wpt2 = sum(squared(wpt));
        if (wpt2 > 1e-12) {
            if (player_rb.avatar_controller_ != nullptr) {
                player_rb.avatar_controller_->increment_legs_z((FixedArray<double, 3>{wpt(0), 0., wpt(1)} / std::sqrt(wpt2)).casted<float>());
                // player_rb.avatar_controller_->increment_legs_z(FixedArray<float, 3>{0.f, 0.f, -1.f});
                if (player_.target_rb_ == nullptr) {
                    // Rotate waypoint back to global coordinates.
                    auto wpt0 = dot(wpt, m);
                    player_rb.avatar_controller_->set_target_yaw((float)std::atan2(-wpt0(0), -wpt0(1)));
                }
                player_rb.avatar_controller_->apply();
                return;
            } else {
                if (wpt(1) > 0) {
                    // The waypoint is behind us => full, inverted steering.
                    if (wpt(0) < 0) {
                        player_.car_movement.steer_left_full();
                        player_rb.vehicle_controller().apply();
                        return;
                    } else {
                        player_.car_movement.steer_right_full();
                        player_rb.vehicle_controller().apply();
                        return;
                    }
                } else {
                    // The waypoint is in front of us => partial, inverted steering.
                    float angle = (float)std::atan(std::abs(wpt(0) / wpt(1)));
                    if (wpt(0) < 0) {
                        player_.car_movement.steer_left_partial(angle);
                        player_rb.vehicle_controller().apply();
                        return;
                    } else {
                        player_.car_movement.steer_right_partial(angle);
                        player_rb.vehicle_controller().apply();
                        return;
                    }
                }
            }
        }
    }
    if (player_rb.avatar_controller_ != nullptr) {
        player_rb.avatar_controller_->apply();
    } else {
        player_.car_movement.steer(0.f);
        player_rb.vehicle_controller().apply();
    }
}

void SingleWaypoint::notify_set_waypoints(size_t nwaypoints) {
    last_visited_ = std::vector<std::chrono::time_point<std::chrono::steady_clock>>(nwaypoints);
    waypoint_id_ = SIZE_MAX;
    nwaypoints_reached_ = 0;
}

void SingleWaypoint::notify_spawn() {
    for (auto& l : last_visited_) {
        l = std::chrono::time_point<std::chrono::steady_clock>();
    }
    set_waypoint(fixed_nans<double, 3>());
    nwaypoints_reached_ = 0;
    waypoint_history_.clear();
}

void SingleWaypoint::draw_waypoint_history(const std::string& filename) const {
    player_.delete_node_mutex_.notify_reading();
    if (!record_waypoints_) {
        THROW_OR_ABORT("draw_waypoint_history but recording is not enabled");
    }
    std::ofstream ofstr{filename};
    if (ofstr.fail()) {
        THROW_OR_ABORT("Could not open \"" + filename + "\" for write");
    }
    Svg<double> svg{ofstr, 600, 600};
    std::vector<double> x;
    std::vector<double> y;
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
        THROW_OR_ABORT("Could not write to file \"" + filename + '"');
    }
}

bool SingleWaypoint::waypoint_defined() const {
    return !any(Mlib::isnan(waypoint_));
}

bool SingleWaypoint::waypoint_reached() const {
    return waypoint_reached_;
}

size_t SingleWaypoint::target_waypoint_id() const {
    return waypoint_id_;
}

size_t SingleWaypoint::previous_waypoint_id() const {
    return previous_waypoint_id_;
}
