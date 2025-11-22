#include "Calculate_Terrain_Spawn_Points.hpp"
#include <Mlib/Geometry/Exceptions/Point_Exception.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Bijection.hpp>
#include <Mlib/Navigation/Sample_SoloMesh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Ground_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>

using namespace Mlib;

void Mlib::calculate_terrain_spawn_points(
    std::list<SpawnPoint>& spawn_points,
    const std::list<Building>& spawn_lines,
    const std::map<std::string, Node>& nodes,
    const GroundBvh& ground_bvh,
    const Bijection<FixedArray<double, 3, 3>>* to_meters,
    const Sample_SoloMesh* ssm)
{
    if (!ssm != !to_meters) {
        THROW_OR_ABORT("Inconsistent to-meters mapping and navmesh parameters");
    }
    for (const Building& bu : spawn_lines) {
        auto team = bu.way.tags.get("spawn:team", "");
        auto group = bu.way.tags.get("spawn:group", "");
        for (auto it = bu.way.nd.begin(); it != bu.way.nd.end(); ++it) {
            auto next = it;
            ++next;
            if (next != bu.way.nd.end()) {
                FixedArray<CompressedScenePos, 2> p = (nodes.at(*it).position + nodes.at(*next).position) / 2;
                FixedArray<ScenePos, 2> dir = funpack(nodes.at(*it).position - nodes.at(*next).position);
                ScenePos len2 = sum(squared(dir));
                if (len2 < 1e-12) {
                    throw PointException{ p, "Spawn direction too small" };
                }
                dir /= std::sqrt(len2);
                CompressedScenePos height;
                if (!ground_bvh.max_height(height, p)) {
                    throw PointException{ p, "Spawn line out of bounds" };
                }
                auto p3 = FixedArray<CompressedScenePos, 3>{p(0), p(1), height};
                if (ssm != nullptr) {
                    auto pm = dot1d(to_meters->model, funpack(p3));
                    auto pc = ssm->closest_point_on_navmesh(pm.casted<float>());
                    if (!pc.has_value()) {
                        throw PointException{ p, "Could not find closest spawn point on navmesh" };
                    }
                    p3 = dot1d(to_meters->view, pc->position.casted<double>()).casted<CompressedScenePos>();
                }
                spawn_points.push_back(SpawnPoint{
                    .type = SpawnPointType::SPAWN_LINE,
                    .location = WayPointLocation::UNKNOWN,
                    .trafo = {
                        tait_bryan_angles_2_matrix<SceneDir>({0.f, 0.f, (SceneDir)std::atan2(dir(0), -dir(1))}),
                        p3},
                    .team = team,
                    .group = group});
            }
        }
    }
}
