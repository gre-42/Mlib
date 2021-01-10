#include "Calculate_Spawn_Points.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Render/Renderables/Renderable_Osm_Map/Renderable_Osm_Map_Helpers.hpp>
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
        SpawnPoint sp;
        sp.location = r.location;
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
        FixedArray<float, 3, 3> R{
            x(0), y(0), z(0),
            x(1), y(1), z(1),
            x(2), y(2), z(2)};
        sp.rotation = matrix_2_tait_bryan_angles(R);
        auto create_spawn_point = [&spawn_points, &sp, &r](SpawnPointType spawn_point_type, float alpha){
            sp.type = spawn_point_type;
            sp.position = alpha * (r.rectangle(0, 0) + r.rectangle(1, 0)) / 2.f + (1 - alpha) * (r.rectangle(0, 1) + r.rectangle(1, 1)) / 2.f;
            spawn_points.push_back(sp);
        };
        if (driving_direction == DrivingDirection::CENTER) {
            create_spawn_point(SpawnPointType::ROAD, 0.5);
        } else if (driving_direction == DrivingDirection::LEFT) {
            if (r.nlanes == 2) {
                create_spawn_point(SpawnPointType::ROAD, 1 - 0.5 - 0.25);
            } else if (r.nlanes == 4) {
                create_spawn_point(SpawnPointType::ROAD, 1 - 0.5 - 0.125);
                create_spawn_point(SpawnPointType::PARKING, 1 - 0.75 - 0.125);
            } else {
                throw std::runtime_error("Unsupported number of lanes");
            }
        } else if (driving_direction == DrivingDirection::RIGHT) {
            create_spawn_point(SpawnPointType::ROAD, 0.75);
            if (r.nlanes == 2) {
                create_spawn_point(SpawnPointType::ROAD, 0.5 + 0.25);
            } else if (r.nlanes == 4) {
                create_spawn_point(SpawnPointType::ROAD, 0.5 + 0.125);
                create_spawn_point(SpawnPointType::PARKING, 0.75 + 0.125);
            } else {
                throw std::runtime_error("Unsupported number of lanes");
            }
        } else {
            throw std::runtime_error("Unknown driving direction");
        }
    }
}
