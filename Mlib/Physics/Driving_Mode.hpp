#pragma once
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>
#include <map>
#include <string>

namespace Mlib {

struct DrivingMode {
    float rest_radius;
    float max_velocity;
    float max_delta_velocity_break;
    float collision_avoidance_radius_break;
    float collision_avoidance_radius_correct;
    float collision_avoidance_cos;
    float collision_avoidance_delta;
    float stuck_velocity;
    float stuck_seconds;
    float unstuck_seconds;
    WayPointLocation way_point_location;
};

static std::map<std::string, DrivingMode> driving_modes{
    {"pedestrian", {
        .rest_radius = 5,
        .max_velocity = 5 / 3.6,
        .max_delta_velocity_break = 1 / 3.6,
        .collision_avoidance_radius_break = 0,
        .collision_avoidance_radius_correct = 0,
        .collision_avoidance_cos = 0.6,
        .collision_avoidance_delta = 0.5,
        .stuck_velocity = 2 / 3.6,
        .stuck_seconds = 6,
        .unstuck_seconds = 5,
        .way_point_location = WayPointLocation::SIDEWALK}},
    {"car_city", {
        .rest_radius = 5,
        .max_velocity = 30 / 3.6,
        .max_delta_velocity_break = 7 / 3.6,
        .collision_avoidance_radius_break = 0,
        .collision_avoidance_radius_correct = 0,
        .collision_avoidance_cos = 0.6,
        .collision_avoidance_delta = 0.5,
        .stuck_velocity = 2 / 3.6,
        .stuck_seconds = 6,
        .unstuck_seconds = 5,
        .way_point_location = WayPointLocation::STREET}},
    {"car_arena", {
        .rest_radius = 30,
        .max_velocity = 70 / 3.6,
        .max_delta_velocity_break = 7 / 3.6,
        .collision_avoidance_radius_break = 20,
        .collision_avoidance_radius_correct = 100,
        .collision_avoidance_cos = 0.6,
        .collision_avoidance_delta = 0.5,
        .stuck_velocity = 2 / 3.6,
        .stuck_seconds = 3,
        .unstuck_seconds = 5,
        .way_point_location = WayPointLocation::STREET}}};

}
