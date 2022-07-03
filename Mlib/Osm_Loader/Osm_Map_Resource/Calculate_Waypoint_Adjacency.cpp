#include "Calculate_Waypoint_Adjacency.hpp"
#include <Mlib/Geometry/Mesh/Edge_Exception.hpp>
#include <Mlib/Geometry/Mesh/Interpolated_Intermediate_Points_Creator.hpp>
#include <Mlib/Geometry/Mesh/Point_Exception.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency_Impl.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Navigation/Sample_SoloMesh.hpp>
#include <Mlib/Navigation/Shortest_Path_Intermediate_Points_Creator.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Ground_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Way_Points.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Vertex_Way_Point.hpp>

using namespace Mlib;

void Mlib::calculate_waypoint_adjacency(
    PointsAndAdjacency<double, 3>& way_points,
    const std::list<TerrainWayPoints>& terrain_way_point_lines,
    const std::list<std::pair<StreetWayPoint, StreetWayPoint>>& street_way_point_edge_descriptors,
    const std::map<std::string, Node>& nodes,
    const GroundBvh& ground_bvh,
    const FixedArray<double, 3, 3>* to_meters,
    const Sample_SoloMesh* ssm,
    double scale)
{
    std::map<std::string, size_t> indices_terrain_wpts;
    for (const TerrainWayPoints& wps : terrain_way_point_lines) {
        for (const std::string& n : wps.way.nd) {
            indices_terrain_wpts.insert({n, indices_terrain_wpts.size()});
        }
    }
    std::map<OrderableFixedArray<double, 3>, size_t> indices_street_wpts;
    for (const auto& e : street_way_point_edge_descriptors) {
        auto p0 = e.first.position();
        auto p1 = e.second.position();
        indices_street_wpts.insert({OrderableFixedArray<double, 3>{ p0 }, indices_street_wpts.size()});
        indices_street_wpts.insert({OrderableFixedArray<double, 3>{ p1 }, indices_street_wpts.size()});
    }
    way_points.points.resize(indices_terrain_wpts.size() + indices_street_wpts.size());
    std::set<OrderableFixedArray<double, 2>> terrain_way_points;
    auto terrain_wpt_p2_to_p3 = [&way_points, &ground_bvh](const FixedArray<double, 2>& p2){
        double height;
        if (ground_bvh.height(height, p2)) {
            return FixedArray<double, 3>{p2(0), p2(1), height};
        } else {
            throw PointException<double, 2>{ p2, "Could not determine height of original waypoint" };
        }
    };
    for (const auto& p : indices_terrain_wpts) {
        auto p2 = nodes.at(p.first).position;
        way_points.points[p.second] = terrain_wpt_p2_to_p3(p2);
        terrain_way_points.insert(OrderableFixedArray<double, 2>{ p2(0), p2(1) });
    }
    for (const auto& p : indices_street_wpts) {
        way_points.points[indices_terrain_wpts.size() + p.second] = p.first;
    }
    way_points.adjacency = SparseArrayCcs<double>{ArrayShape{
        indices_terrain_wpts.size() + indices_street_wpts.size(),
        indices_terrain_wpts.size() + indices_street_wpts.size()}};
    
    {
        auto insert_edge_1_lane = [&way_points, &nodes, &indices_terrain_wpts](const std::string& a, const std::string& b, WayPointsOrientation orientation) {
            double dist = std::sqrt(sum(squared(nodes.at(a).position - nodes.at(b).position)));
            if (!way_points.adjacency.column(indices_terrain_wpts.at(a)).insert({indices_terrain_wpts.at(b), dist}).second) {
                throw std::runtime_error("Could not insert waypoint (0)");
            }
            if (orientation == WayPointsOrientation::BIDIRECTIONAL) {
                if (!way_points.adjacency.column(indices_terrain_wpts.at(b)).insert({indices_terrain_wpts.at(a), dist}).second) {
                    throw std::runtime_error("Could not insert waypoint (1)");
                }
            }
        };
        for (const TerrainWayPoints& wps : terrain_way_point_lines) {
            for (auto it = wps.way.nd.begin(); it != wps.way.nd.end(); ++it) {
                auto s = it;
                ++s;
                if (s != wps.way.nd.end()) {
                    insert_edge_1_lane(*it, *s, wps.orientation);
                }
            }
        }
        for (size_t i = 0; i < indices_terrain_wpts.size(); ++i) {
            if (!way_points.adjacency.column(i).insert({i, 0.f}).second) {
                throw std::runtime_error("Could not insert waypoint (2)");
            }
        }
    }
    {
        for (const auto& e : street_way_point_edge_descriptors) {
            auto p0 = e.first.position();
            auto p1 = e.second.position();
            float dist = std::sqrt(sum(squared(p0 - p1)));
            size_t col_id_0 = indices_terrain_wpts.size() + indices_street_wpts.at(OrderableFixedArray{ p0 });
            size_t col_id_1 = indices_terrain_wpts.size() + indices_street_wpts.at(OrderableFixedArray{ p1 });
            if (!way_points.adjacency.column(col_id_0).insert({col_id_1, dist}).second) {
                throw std::runtime_error("Could not insert waypoint (3)");
            }
        }
        for (size_t i = 0; i < indices_street_wpts.size(); ++i) {
            if (!way_points.adjacency.column(indices_terrain_wpts.size() + i).insert({indices_terrain_wpts.size() + i, 0.f}).second) {
                throw std::runtime_error("Could not insert waypoint (4)");
            }
        }
    }
    auto interplator = [&ground_bvh, &terrain_way_points](
        const FixedArray<double, 3>& p0,
        const FixedArray<double, 3>& p1,
        double a0,
        double a1)
    {
        FixedArray<double, 3> res = p0 * a0 + p1 * a1;
        if (terrain_way_points.contains(OrderableFixedArray<double, 2>{ p0(0), p0(1) }) ||
            terrain_way_points.contains(OrderableFixedArray<double, 2>{ p1(0), p1(1) }))
        {
            if (!ground_bvh.height(res(2), FixedArray<double, 2>{ res(0), res(1) })) {
                throw PointException<double, 3>{ res, "Could not determine height of interpolated waypoint" };
            }
        }
        return res;
    };
    if (!ssm != !to_meters) {
        throw std::runtime_error("Inconsistent to-meters mapping an navmesh parameters");
    }
    if (ssm != nullptr) {
        std::map<OrderableFixedArray<float, 3>, dtPolyRef> poly_refs;
        for (auto& p : way_points.points) {
            LocalizedNavmeshNode lp = ssm->closest_point_on_navmesh(dot1d(*to_meters, p).casted<float>());
            if (any(isnan(lp.position))) {
                throw std::runtime_error("Could not find closest point on navmesh");
            }
            if (!poly_refs.insert({ OrderableFixedArray{lp.position}, lp.polyRef }).second) {
                throw std::runtime_error("Found duplicate waypoint");
            }
            p = lp.position.casted<double>();
        }
        way_points.update_adjacency();
        auto itm = inv(*to_meters).value();
        try {
            way_points.subdivide(
                ShortestPathIntermediatePointsCreator{*ssm, poly_refs, 2.f},
                SubdivisionType::MAKE_SYMMETRIC);
        } catch (const EdgeException<double>& e) {
            throw EdgeException<double>{dot1d(itm, e.a), dot1d(itm, e.b), e.what()};
        }
        for (auto& p : way_points.points) {
            p = dot1d(itm, p);
            if (!ground_bvh.height3d(p(2), p)) {
                throw PointException<double, 3>{ p, "Could not determine height of shortest-path waypoint" };
            }
        }
        way_points.update_adjacency();
    } else {
        // Not subdividing in case of pathfinding because
        // the "terrain_way_points" array is not (yet) updated
        // in this case.
        way_points.subdivide(
            InterpolatedIntermediatePointsCreator<double, 3, decltype(interplator)>(
                50. * scale,
                interplator),
            SubdivisionType::SYMMETRIC);
    }
}
