#include "Calculate_Spawn_Points.hpp"
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Rectangle.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

void Mlib::calculate_spawn_points(
    std::list<SpawnPoint>& spawn_points,
    const std::list<StreetRectangle>& street_rectangles,
    double scale,
    DrivingDirection driving_direction)
{
    for (const auto& r : street_rectangles) {
        auto rectangle = funpack(r.rectangle);
        FixedArray<ScenePos, 3> x = rectangle[0][0] - rectangle[0][1];
        FixedArray<ScenePos, 3> y = rectangle[0][0] - rectangle[1][0];
        double ly;
        {
            double lx2 = sum(squared(x));
            double ly2 = sum(squared(y));
            if ((r.location == WayPointLocation::STREET) && (lx2 < squared(3 * scale))) {
                continue;
            }
            if (ly2 < squared(3 * scale)) {
                continue;
            }
            ly = std::sqrt(ly2);
            x /= std::sqrt(lx2);
            y /= ly;
            x -= y * dot0d(y, x);
            x /= std::sqrt(sum(squared(x)));
        }
        FixedArray<double, 3> z = cross(x, y);
        auto R0 = FixedArray<double, 3, 3>::init(
            x(0), y(0), z(0),
            x(1), y(1), z(1),
            x(2), y(2), z(2));
        auto r0 = matrix_2_tait_bryan_angles(R0).casted<float>();
        auto r1 = matrix_2_tait_bryan_angles(dot2d(rodrigues2(z, M_PI), R0)).casted<float>();
        auto create_spawn_point = [&](
            SpawnPointType spawn_point_type,
            double alpha,
            const FixedArray<float, 3>& rotation)
        {
            for (double beta = 0; beta < 1; beta += (10 * scale) / ly) {
                auto position =
                    (alpha * (beta * rectangle[0][0] + (1. - beta) * rectangle[1][0]) +
                    (1. - alpha) * (beta * rectangle[0][1] + (1. - beta) * rectangle[1][1])).casted<CompressedScenePos>();
                spawn_points.push_back(SpawnPoint{
                    .type = spawn_point_type,
                    .location = r.location,
                    .trafo = { tait_bryan_angles_2_matrix(rotation), position }});
            }
        };
        if (driving_direction == DrivingDirection::CENTER) {
            create_spawn_point(SpawnPointType::ROAD, 0.5, r0);
        } else if (driving_direction == DrivingDirection::LEFT) {
            if (r.road_properties.nlanes == 1) {
                // Do nothing
            } else if (r.road_properties.nlanes == 2 || r.road_properties.nlanes == 3) {
                create_spawn_point(SpawnPointType::ROAD, 0.5 - 0.25, r0);
                create_spawn_point(SpawnPointType::ROAD, 0.5 + 0.25, r1);
            } else if (r.road_properties.nlanes >= 4) {
                create_spawn_point(SpawnPointType::ROAD, 0.5 - 0.125, r0);
                create_spawn_point(SpawnPointType::PARKING, 0.5 - 0.25 - 0.125, r0);
                create_spawn_point(SpawnPointType::ROAD, 0.5 + 0.125, r1);
                create_spawn_point(SpawnPointType::PARKING, 0.5 + 0.25 + 0.125, r1);
            } else {
                THROW_OR_ABORT("Unsupported number of lanes: " + std::to_string(r.road_properties.nlanes));
            }
        } else if (driving_direction == DrivingDirection::RIGHT) {
            if (r.road_properties.nlanes == 1) {
                // Do nothing
            } else if (r.road_properties.nlanes == 2 || r.road_properties.nlanes == 3) {
                create_spawn_point(SpawnPointType::ROAD, 0.5 + 0.25, r0);
                create_spawn_point(SpawnPointType::ROAD, 0.5 - 0.25, r1);
            } else if (r.road_properties.nlanes >= 4) {
                create_spawn_point(SpawnPointType::ROAD, 0.5 + 0.125, r0);
                create_spawn_point(SpawnPointType::PARKING, 0.5 + 0.25 + 0.125, r0);
                create_spawn_point(SpawnPointType::ROAD, 0.5 - 0.125, r1);
                create_spawn_point(SpawnPointType::PARKING, 0.5 - 0.25 - 0.125, r1);
            } else {
                THROW_OR_ABORT("Unsupported number of lanes: " + std::to_string(r.road_properties.nlanes));
            }
        } else {
            THROW_OR_ABORT("Unknown driving direction");
        }
    }
}
