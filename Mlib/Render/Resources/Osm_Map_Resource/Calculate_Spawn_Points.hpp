#pragma once
#include <list>

namespace Mlib {

enum class DrivingDirection;
struct SpawnPoint;
struct StreetRectangle;

void calculate_spawn_points(
    std::list<SpawnPoint>& spawn_points,
    const std::list<StreetRectangle>& street_rectangles,
    float scale,
    DrivingDirection driving_direction);

}
