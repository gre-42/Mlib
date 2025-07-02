#include "Single_Waypoint.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Graph/Point_And_Flags.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Ai/Ai_Waypoint.hpp>
#include <Mlib/Physics/Ai/IVehicle_Ai.hpp>
#include <Mlib/Physics/Physics_Engine/Beacons.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Scene_Vehicle/Scene_Vehicle.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static const auto BEACON = VariableAndHash<std::string>{"beacon"};

SingleWaypoint::SingleWaypoint(const DanglingBaseClassRef<Player>& player)
    : player_{ player }
    , target_velocity_{ NAN }
    , waypoint_{ std::nullopt }
    , waypoint_id_{ SIZE_MAX }
    , previous_waypoint_id_{ SIZE_MAX }
    , waypoint_reached_{ false }
    , nwaypoints_reached_{ 0 }
    , max_waypoint_history_length_{ 10 }
{}

SingleWaypoint::~SingleWaypoint() = default;

void SingleWaypoint::set_target_velocity(float v) {
    target_velocity_ = v;
}

void SingleWaypoint::set_waypoint_internal(const std::optional<WayPoint>& waypoint, size_t waypoint_id) {
    previous_waypoint_id_ = waypoint_id_;
    if (!waypoint.has_value()) {
        waypoint_ = { { (CompressedScenePos)4.f, (CompressedScenePos)2.f, (CompressedScenePos)42.f }, WayPointLocation::NONE };
    }
    waypoint_ = waypoint;
    waypoint_id_ = waypoint_id;
    if (has_waypoint()) {
        if (max_waypoint_history_length_ > 0) {
            waypoint_history_.push_back(*waypoint);
            if (waypoint_history_.size() > max_waypoint_history_length_) {
                waypoint_history_.pop_front();
            }
        }
    }
    waypoint_reached_ = false;
}

void SingleWaypoint::set_waypoint(const WayPoint& waypoint, size_t waypoint_id) {
    set_waypoint_internal(waypoint, waypoint_id);
}

void SingleWaypoint::set_waypoint(const WayPoint& waypoint) {
    set_waypoint_internal(waypoint, SIZE_MAX);
}

void SingleWaypoint::clear_waypoint() {
    set_waypoint_internal(std::nullopt, SIZE_MAX);
}

void SingleWaypoint::move_to_waypoint(
    const SkillMap& skills,
    const StaticWorld& world)
{
    if (getenv_default_bool("DRAW_WAYPOINT_HISTORY", false)) {
        if (waypoint_.has_value()) {
            add_beacon(Beacon::create(funpack(waypoint_->position), BEACON));
        }
        for (const auto& w : waypoint_history_) {
            add_beacon(Beacon::create(funpack(w.position), BEACON));
        }
    }
    player_->delete_node_mutex_.assert_this_thread_is_deleter_thread();
    if (!player_->has_scene_vehicle()) {
        return;
    }
    auto rb = player_->rigid_body();
    if (any(rb->move_to(
        AiWaypoint{
            waypoint_,                  // position_of_destination
            fixed_zeros<float, 3>(),    // velocity_of_destination
            std::nullopt,               // velocity_at_destination
            &waypoint_history_          // waypoint_history
        },
        &skills,
        world) & VehicleAiMoveToStatus::WAYPOINT_REACHED))
    {
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
    clear_waypoint();
    nwaypoints_reached_ = 0;
    waypoint_history_.clear();
}

void SingleWaypoint::draw_waypoint_history(const std::string& filename) const {
    if (max_waypoint_history_length_ == 0) {
        THROW_OR_ABORT("draw_waypoint_history but recording is not enabled");
    }
    std::ofstream ofstr{filename};
    if (ofstr.fail()) {
        THROW_OR_ABORT("Could not open \"" + filename + "\" for write");
    }
    Svg<ScenePos> svg{ ofstr, 600, 600 };
    std::vector<CompressedScenePos> x;
    std::vector<CompressedScenePos> y;
    x.resize(waypoint_history_.size());
    y.resize(waypoint_history_.size());
    size_t i = 0;
    for (const auto& w : waypoint_history_) {
        x[i] = w.position(0);
        y[i] = w.position(2);
        ++i;
    }
    svg.plot(x, y);
    svg.finish();
    ofstr.flush();
    if (ofstr.fail()) {
        THROW_OR_ABORT("Could not write to file \"" + filename + '"');
    }
}

bool SingleWaypoint::has_waypoint() const {
    return waypoint_.has_value();
}

bool SingleWaypoint::waypoint_reached() const {
    return waypoint_reached_;
}

const SingleWaypoint::WayPoint& SingleWaypoint::get_waypoint() const {
    if (!has_waypoint()) {
        THROW_OR_ABORT("Waypoint not defined");
    }
    if (!waypoint_.has_value()) {
        verbose_abort("Internal error in SingleWaypoint::get_waypoint");
    }
    return *waypoint_;
}

size_t SingleWaypoint::nwaypoints_reached() const {
    return nwaypoints_reached_;
}

size_t SingleWaypoint::target_waypoint_id() const {
    return waypoint_id_;
}

size_t SingleWaypoint::previous_waypoint_id() const {
    return previous_waypoint_id_;
}

std::chrono::steady_clock::time_point SingleWaypoint::last_visited(size_t waypoint_id) const {
    if (waypoint_id >= last_visited_.size()) {
        THROW_OR_ABORT("Waypoint ID too large");
    }
    return last_visited_.at(waypoint_id);
}
