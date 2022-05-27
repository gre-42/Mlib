#pragma once
#include <map>
#include <string>

namespace Mlib {

enum class WayPointLocation;

struct DrivingMode {
    float waypoint_reached_radius;
    float rest_radius;
    float lookahead_velocity;
    float max_velocity;
    float max_delta_velocity_brake;
    float collision_avoidance_radius_brake;
    float collision_avoidance_radius_correct;
    float collision_avoidance_cos;
    float collision_avoidance_delta;
    float stuck_velocity;
    float stuck_seconds;
    float unstuck_seconds;
    WayPointLocation way_point_location;
};

extern std::map<std::string, DrivingMode> driving_modes;

}
