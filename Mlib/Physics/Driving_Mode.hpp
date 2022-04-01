#pragma once
#include <Mlib/Physics/Units.hpp>
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
        .rest_radius = 5 * meters,
        .max_velocity = 5 * kph,
        .max_delta_velocity_break = 1 * kph,
        .collision_avoidance_radius_break = 0 * meters,
        .collision_avoidance_radius_correct = 0 * meters,
        .collision_avoidance_cos = 0.6f,
        .collision_avoidance_delta = 0.5f,
        .stuck_velocity = 2 * kph,
        .stuck_seconds = 6,
        .unstuck_seconds = 5,
        .way_point_location = WayPointLocation::SIDEWALK}},
    {"car_city", {
        .rest_radius = 5 * meters,
        .max_velocity = 30 * kph,
        .max_delta_velocity_break = 7 * kph,
        .collision_avoidance_radius_break = 0 * meters,
        .collision_avoidance_radius_correct = 0 * meters,
        .collision_avoidance_cos = 0.6f,
        .collision_avoidance_delta = 0.5f,
        .stuck_velocity = 2 * kph,
        .stuck_seconds = 6,
        .unstuck_seconds = 5,
        .way_point_location = WayPointLocation::STREET}},
    {"car_arena", {
        .rest_radius = 10 * meters,
        .max_velocity = 70 * kph,
        .max_delta_velocity_break = 7 * kph,
        .collision_avoidance_radius_break = 20 * meters,
        .collision_avoidance_radius_correct = 100 * meters,
        .collision_avoidance_cos = 0.6f,
        .collision_avoidance_delta = 0.5f,
        .stuck_velocity = 2 * kph,
        .stuck_seconds = 3,
        .unstuck_seconds = 5,
        .way_point_location = WayPointLocation::EXPLICIT}}};

}
