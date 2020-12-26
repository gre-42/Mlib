#pragma once
#include <map>
#include <string>

namespace Mlib {

struct DrivingMode {
    float rest_radius;
    float max_velocity;
    float max_velocity_break;
    float collision_avoidance_radius_break;
    float collision_avoidance_radius_correct;
    float collision_avoidance_cos;
    float collision_avoidance_delta;
    float stuck_velocity;
    float stuck_seconds;
    float unstuck_seconds;
};

static std::map<std::string, DrivingMode> driving_modes{
    {"city", {
        .rest_radius = 2,
        .max_velocity = 30 / 3.6,
        .max_velocity_break = 2,
        .collision_avoidance_radius_break = 2,
        .collision_avoidance_radius_correct = 2,
        .collision_avoidance_cos = 0.6,
        .collision_avoidance_delta = 0.5,
        .stuck_velocity = 2 / 3.6,
        .stuck_seconds = 3,
        .unstuck_seconds = 5}},
    {"arena", {
        .rest_radius = 30,
        .max_velocity = 70 / 3.6,
        .max_velocity_break = 2,
        .collision_avoidance_radius_break = 20,
        .collision_avoidance_radius_correct = 100,
        .collision_avoidance_cos = 0.6,
        .collision_avoidance_delta = 0.5,
        .stuck_velocity = 2 / 3.6,
        .stuck_seconds = 3,
        .unstuck_seconds = 5}}};

}
