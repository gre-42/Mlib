#include "Calculate_Spawn_Points.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Render/Resources/Renderable_Osm_Map/Renderable_Osm_Map_Helpers.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>
#include <stdexcept>

using namespace Mlib;

void Mlib::calculate_spawn_points(
    std::list<SpawnPoint>& spawn_points,
    const std::list<StreetRectangle>& street_rectangles,
    float scale,
    DrivingDirection driving_direction)
{
    for (const auto& r : street_rectangles) {
        FixedArray<float, 3> x = r.rectangle(0, 0) - r.rectangle(0, 1);
        FixedArray<float, 3> y = r.rectangle(0, 0) - r.rectangle(1, 0);
        {
            float lx2 = sum(squared(x));
            float ly2 = sum(squared(y));
            if ((r.location == WayPointLocation::STREET) && (lx2 < squared(3 * scale))) {
                continue;
            }
            if (ly2 < squared(3 * scale)) {
                continue;
            }
            x /= std::sqrt(lx2);
            y /= std::sqrt(ly2);
            x -= y * dot0d(y, x);
            x /= std::sqrt(sum(squared(x)));
        }
        FixedArray<float, 3> z = cross(x, y);
        FixedArray<float, 3, 3> R0{
            x(0), y(0), z(0),
            x(1), y(1), z(1),
            x(2), y(2), z(2)};
        auto r0 = matrix_2_tait_bryan_angles(R0);
        auto r1 = matrix_2_tait_bryan_angles(dot2d(rodrigues(z, float(M_PI)), R0));
        auto create_spawn_point = [&spawn_points, &r](
            SpawnPointType spawn_point_type,
            float alpha,
            const FixedArray<float, 3>& rotation)
        {
            spawn_points.push_back(SpawnPoint{
                .type = spawn_point_type,
                .location = r.location,
                .position = alpha * (r.rectangle(0, 0) + r.rectangle(1, 0)) / 2.f + (1 - alpha) * (r.rectangle(0, 1) + r.rectangle(1, 1)) / 2.f,
                .rotation = rotation});
        };
        if (driving_direction == DrivingDirection::CENTER) {
            create_spawn_point(SpawnPointType::ROAD, 0.5, r0);
        } else if (driving_direction == DrivingDirection::LEFT) {
            if (r.nlanes == 2) {
                create_spawn_point(SpawnPointType::ROAD, 0.5 - 0.25, r0);
                create_spawn_point(SpawnPointType::ROAD, 0.5 + 0.25, r1);
            } else if (r.nlanes == 4) {
                create_spawn_point(SpawnPointType::ROAD, 0.5 - 0.125, r0);
                create_spawn_point(SpawnPointType::PARKING, 0.5 - 0.25 - 0.125, r0);
                create_spawn_point(SpawnPointType::ROAD, 0.5 + 0.125, r1);
                create_spawn_point(SpawnPointType::PARKING, 0.5 + 0.25 + 0.125, r1);
            } else {
                throw std::runtime_error("Unsupported number of lanes");
            }
        } else if (driving_direction == DrivingDirection::RIGHT) {
            if (r.nlanes == 2) {
                create_spawn_point(SpawnPointType::ROAD, 0.5 + 0.25, r0);
                create_spawn_point(SpawnPointType::ROAD, 0.5 - 0.25, r1);
            } else if (r.nlanes == 4) {
                create_spawn_point(SpawnPointType::ROAD, 0.5 + 0.125, r0);
                create_spawn_point(SpawnPointType::PARKING, 0.5 + 0.25 + 0.125, r0);
                create_spawn_point(SpawnPointType::ROAD, 0.5 - 0.125, r1);
                create_spawn_point(SpawnPointType::PARKING, 0.5 - 0.25 - 0.125, r1);
            } else {
                throw std::runtime_error("Unsupported number of lanes");
            }
        } else {
            throw std::runtime_error("Unknown driving direction");
        }
    }
}
