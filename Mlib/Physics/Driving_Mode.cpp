#include "Driving_Mode.hpp"
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>

using namespace Mlib;

std::map<std::string, DrivingMode> Mlib::driving_modes{
    {"pedestrian", DrivingMode{
        .waypoint_reached_radius = 5 * meters,
        .rest_radius = 4 * meters,
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
        .waypoint_reached_radius = 5 * meters,
        .rest_radius = 4 * meters,
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
        .waypoint_reached_radius = 10 * meters,
        .rest_radius = 8 * meters,
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
