#pragma once
#include <map>
#include <string>

#if defined(_MSC_VER) && defined(MlibPhysics_EXPORTS)
#define MLIB_PHYSICS_API __declspec(dllexport)
#else
#define MLIB_PHYSICS_API __declspec(dllimport)
#endif

namespace Mlib {

enum class WayPointLocation;

struct DrivingMode {
    float waypoint_ofs;
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

MLIB_PHYSICS_API extern std::map<std::string, DrivingMode> driving_modes;

}
