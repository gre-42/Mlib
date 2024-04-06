#include "Single_Waypoint.hpp"
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Scene_Vehicle/Control_Source.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Players/Vehicle_Ai/IVehicle_Ai.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

// namespace Mlib { thread_local extern std::list<Beacon> g_beacons; }

SingleWaypoint::SingleWaypoint(Player& player)
    : player_{ player }
    , target_velocity_{ NAN }
    , waypoint_{ fixed_nans <double, 3>() }
    , waypoint_id_{ SIZE_MAX }
    , previous_waypoint_id_{ SIZE_MAX }
    , waypoint_reached_{ false }
    , nwaypoints_reached_{ 0 }
    , record_waypoints_{ false }
{}

SingleWaypoint::~SingleWaypoint() = default;

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
    if (any(player_.vehicle_ai().move_to(waypoint_, std::nullopt) & VehicleAiMoveToStatus::DESTINATION_REACHED)) {
        if (waypoint_id_ != SIZE_MAX) {
            last_visited_.at(waypoint_id_) = std::chrono::steady_clock::now();
        }
        waypoint_reached_ = true;
        ++nwaypoints_reached_;
    }
}

void SingleWaypoint::notify_set_waypoints(size_t nwaypoints) {
    last_visited_ = std::vector<std::chrono::steady_clock::time_point>(nwaypoints);
    waypoint_id_ = SIZE_MAX;
    nwaypoints_reached_ = 0;
}

void SingleWaypoint::notify_spawn() {
    for (auto& l : last_visited_) {
        l = std::chrono::steady_clock::time_point();
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
