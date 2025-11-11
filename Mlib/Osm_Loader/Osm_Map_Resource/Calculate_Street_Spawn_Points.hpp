#pragma once
#include <cstdint>
#include <list>

namespace Mlib {

enum class DrivingDirection: uint32_t;
struct SpawnPoint;
struct StreetRectangle;

void calculate_street_spawn_points(
    std::list<SpawnPoint>& spawn_points,
    const std::list<StreetRectangle>& street_rectangles,
    double scale,
    DrivingDirection driving_direction);

}
