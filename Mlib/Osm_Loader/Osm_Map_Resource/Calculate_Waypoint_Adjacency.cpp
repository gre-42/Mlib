#include "Calculate_Waypoint_Adjacency.hpp"
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>
#include <Mlib/Geometry/Exceptions/Point_Exception.hpp>
#include <Mlib/Geometry/Mesh/Interpolated_Intermediate_Points_Creator.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency_Impl.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Navigation/Sample_SoloMesh.hpp>
#include <Mlib/Navigation/Shortest_Path_Intermediate_Points_Creator.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Ground_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Way_Point.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Way_Points.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <set>

using namespace Mlib;

void Mlib::calculate_waypoint_adjacency(
    PointsAndAdjacency<double, 3>& way_points,
    const std::list<TerrainWayPoints>& raw_terrain_way_point_lines,
    WayPointsClass terrain_way_point_filter,
    const std::list<std::pair<StreetWayPoint, StreetWayPoint>>& street_way_point_edge_descriptors,
    const std::map<std::string, Node>& nodes,
    const GroundBvh& ground_bvh,
    const FixedArray<double, 3, 3>* to_meters,
    const Sample_SoloMesh* ssm,
    double scale)
{
    std::list<const TerrainWayPoints*> filtered_terrain_way_point_lines;
    for (const TerrainWayPoints& wps : raw_terrain_way_point_lines) {
        if (any(wps.class_ & terrain_way_point_filter)) {
            filtered_terrain_way_point_lines.push_back(&wps);
        }
    }
    std::map<std::string, size_t> indices_terrain_wpts;
    for (const TerrainWayPoints* wps : filtered_terrain_way_point_lines) {
        for (const std::string& n : wps->way.nd) {
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
    std::set<size_t> terrain_way_points;
    for (const auto& [osm_id, adjacency_id] : indices_terrain_wpts) {
        const auto& node = nodes.at(osm_id);
        auto p2 = node.position;
        auto hwr = parse_height_with_reference(node.tags, "height", "height_reference", osm_id);
        if (hwr.has_value() && hwr.value().reference == HeightReference::WATER) {
            way_points.points[adjacency_id] = FixedArray<double, 3>{p2(0), p2(1), hwr.value().height * scale};
        } else {
            double height;
            if (ground_bvh.height(height, p2)) {
                if (hwr.has_value()) {
                    if (hwr.value().reference != HeightReference::GROUND) {
                        THROW_OR_ABORT(osm_id + ": Unknown height reference, expected \"ground\"");
                    }
                    way_points.points[adjacency_id] = FixedArray<double, 3>{ p2(0), p2(1), height + hwr.value().height * scale };
                } else {
                    way_points.points[adjacency_id] = FixedArray<double, 3>{ p2(0), p2(1), height };
                }
            } else {
                throw PointException<double, 2>{ p2, osm_id + ": Could not determine height of original waypoint" };
            }
            terrain_way_points.insert(adjacency_id);
        }
    }
    for (const auto& [position, adjacency_id_offset] : indices_street_wpts) {
        way_points.points[indices_terrain_wpts.size() + adjacency_id_offset] = position;
    }
    way_points.adjacency = SparseArrayCcs<double>{ArrayShape{
        indices_terrain_wpts.size() + indices_street_wpts.size(),
        indices_terrain_wpts.size() + indices_street_wpts.size()}};
    
    std::set<std::pair<size_t, size_t>> air_way_lines;
    {
        auto insert_edge_1_lane = [&](const std::string& a, const std::string& b, const TerrainWayPoints& wps) {
            double dist = std::sqrt(sum(squared(nodes.at(a).position - nodes.at(b).position)));
            if (!way_points.adjacency.column(indices_terrain_wpts.at(a)).insert({indices_terrain_wpts.at(b), dist}).second) {
                THROW_OR_ABORT("Could not insert waypoint (0)");
            }
            if (wps.class_ == WayPointsClass::AIRWAY) {
                air_way_lines.insert({ indices_terrain_wpts.at(a), indices_terrain_wpts.at(b) });
            }
            if (wps.orientation == WayPointsOrientation::BIDIRECTIONAL) {
                if (!way_points.adjacency.column(indices_terrain_wpts.at(b)).insert({indices_terrain_wpts.at(a), dist}).second) {
                    THROW_OR_ABORT("Could not insert waypoint (1)");
                }
                if (wps.class_ == WayPointsClass::AIRWAY) {
                    air_way_lines.insert({ indices_terrain_wpts.at(b), indices_terrain_wpts.at(a) });
                }
            }
        };
        for (const TerrainWayPoints* wps : filtered_terrain_way_point_lines) {
            for (auto it = wps->way.nd.begin(); it != wps->way.nd.end(); ++it) {
                auto s = it;
                ++s;
                if (s != wps->way.nd.end()) {
                    insert_edge_1_lane(*it, *s, *wps);
                }
            }
        }
        for (size_t i = 0; i < indices_terrain_wpts.size(); ++i) {
            if (!way_points.adjacency.column(i).insert({i, 0.f}).second) {
                THROW_OR_ABORT("Could not insert waypoint (2)");
            }
        }
    }
    {
        for (const auto& e : street_way_point_edge_descriptors) {
            auto p0 = e.first.position();
            auto p1 = e.second.position();
            double dist = std::sqrt(sum(squared(p0 - p1)));
            size_t col_id_0 = indices_terrain_wpts.size() + indices_street_wpts.at(OrderableFixedArray{ p0 });
            size_t col_id_1 = indices_terrain_wpts.size() + indices_street_wpts.at(OrderableFixedArray{ p1 });
            if (!way_points.adjacency.column(col_id_0).insert({col_id_1, dist}).second) {
                THROW_OR_ABORT("Could not insert waypoint (3)");
            }
        }
        for (size_t i = 0; i < indices_street_wpts.size(); ++i) {
            if (!way_points.adjacency.column(indices_terrain_wpts.size() + i).insert({indices_terrain_wpts.size() + i, 0.f}).second) {
                THROW_OR_ABORT("Could not insert waypoint (4)");
            }
        }
    }
    auto interpolator = [&](
        const FixedArray<double, 3> p0,
        const FixedArray<double, 3> p1,
        double a0,
        double a1)
    {
        FixedArray<double, 3> res = p0 * a0 + p1 * a1;
        if (!ground_bvh.height(res(2), FixedArray<double, 2>{ res(0), res(1) })) {
            throw PointException<double, 3>{ res, "Could not determine height of interpolated waypoint" };
        }
        return res;
    };
    if (!ssm != !to_meters) {
        THROW_OR_ABORT("Inconsistent to-meters mapping an navmesh parameters");
    }
    if (ssm != nullptr) {
        std::map<OrderableFixedArray<float, 3>, dtPolyRef> poly_refs;
        for (auto&& [i, p] : enumerate(way_points.points)) {
            p = dot1d(*to_meters, p);
            if (!terrain_way_points.contains(i)) {
                continue;
            }
            LocalizedNavmeshNode lp = ssm->closest_point_on_navmesh(p.casted<float>());
            if (any(Mlib::isnan(lp.position))) {
                throw PointException<double, 3>{ p, "Could not find closest point on navmesh" };
            }
            if (!poly_refs.insert({ OrderableFixedArray{lp.position}, lp.polyRef }).second) {
                throw PointException<double, 3>{ p, "Found duplicate waypoint" };
            }
            p = lp.position.casted<double>();
        }
        way_points.update_adjacency();
        auto oitm = inv(*to_meters);
        if (!oitm.has_value()) {
            THROW_OR_ABORT("Could not compute inverse to_meters mapping");
        }
        const auto& itm = oitm.value();
        ShortestPathIntermediatePointsCreator spipc{*ssm, poly_refs, 2.f};
        try {
            way_points.subdivide(
                [&](size_t r, size_t c, const double& distance) -> std::vector<FixedArray<double, 3>> {
                    if (air_way_lines.contains({r, c})) {
                        return { way_points.points.at(r), way_points.points.at(c) };
                    } else {
                        return spipc(way_points.points.at(r), way_points.points.at(c), distance);
                    }
                },
                SubdivisionType::ASYMMETRIC);
        } catch (const EdgeException<double>& e) {
            throw EdgeException<double>{dot1d(itm, e.a), dot1d(itm, e.b), e.what()};
        } catch (const PointException<double, 3>& e) {
            throw PointException<double, 3>{dot1d(itm, e.point), e.what()};
        }
        for (auto& p : way_points.points) {
            p = dot1d(itm, p);
            if (!ground_bvh.height3d(p(2), p)) {
                throw PointException<double, 3>{ p, "Could not determine height of shortest-path waypoint" };
            }
        }
        way_points.update_adjacency();
    } else {
        InterpolatedIntermediatePointsCreator<double, 3, decltype(interpolator)> terrain_iipc{
            50. * scale,
            interpolator };
        auto idef = interpolate_default<double, 3>;
        InterpolatedIntermediatePointsCreator<double, 3, decltype(idef)> street_iipc{
            50. * scale,
            idef };
        way_points.subdivide(
            [&](size_t r, size_t c, const double& distance) -> std::vector<FixedArray<double, 3>> {
                if (air_way_lines.contains({r, c})) {
                    return { way_points.points.at(r), way_points.points.at(c) };
                } else {
                    if (terrain_way_points.contains(r) || terrain_way_points.contains(c)) {
                        return terrain_iipc(way_points.points.at(r), way_points.points.at(c), distance);
                    } else {
                        return street_iipc(way_points.points.at(r), way_points.points.at(c), distance);
                    }
                }
            },
            SubdivisionType::ASYMMETRIC);
    }
}
