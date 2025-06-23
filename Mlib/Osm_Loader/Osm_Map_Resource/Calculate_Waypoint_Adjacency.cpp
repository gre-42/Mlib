#include "Calculate_Waypoint_Adjacency.hpp"
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>
#include <Mlib/Geometry/Exceptions/Point_Exception.hpp>
#include <Mlib/Geometry/Mesh/Interpolated_Intermediate_Points_Creator.hpp>
#include <Mlib/Geometry/Mesh/Point_And_Flags.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency_Impl.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Bijection.hpp>
#include <Mlib/Navigation/Sample_SoloMesh.hpp>
#include <Mlib/Navigation/Shortest_Path_Intermediate_Points_Creator.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Ground_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Way_Point.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Way_Points.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <set>

using namespace Mlib;

void Mlib::calculate_waypoint_adjacency(
    PointsAndAdjacency<PointAndFlags<FixedArray<CompressedScenePos, 3>, WayPointLocation>>& way_points,
    const std::list<TerrainWayPoints>& raw_terrain_way_point_lines,
    WayPointsClass terrain_way_point_filter,
    const std::list<std::pair<StreetWayPoint, StreetWayPoint>>& street_way_point_edge_descriptors,
    const std::map<std::string, Node>& nodes,
    const GroundBvh& ground_bvh,
    const Bijection<FixedArray<double, 3, 3>>* to_meters,
    const Sample_SoloMesh* ssm,
    double scale,
    double merge_radius,
    double error_radius,
    CompressedScenePos waypoint_distance)
{
    using WayPoint = PointAndFlags<FixedArray<CompressedScenePos, 3>, WayPointLocation>;

    std::list<const TerrainWayPoints*> filtered_terrain_way_point_lines;
    for (const TerrainWayPoints& wps : raw_terrain_way_point_lines) {
        if (any(wps.class_ & terrain_way_point_filter)) {
            filtered_terrain_way_point_lines.push_back(&wps);
        }
    }
    std::map<std::string, std::pair<size_t, WayPointLocation>> indices_terrain_wpts;
    for (const TerrainWayPoints* wps : filtered_terrain_way_point_lines) {
        for (const std::string& n : wps->way.nd) {
            auto it = indices_terrain_wpts.insert({ n, {indices_terrain_wpts.size(), WayPointLocation::NONE} });
            switch (wps->class_) {
            case WayPointsClass::GROUND:
                it.first->second.second |= WayPointLocation::EXPLICIT_GROUND;
                continue;
            case WayPointsClass::AIRWAY:
                it.first->second.second |= WayPointLocation::AIRWAY;
                continue;
            case WayPointsClass::NONE:
                THROW_OR_ABORT("Waypoint-class is NONE");
            }
            THROW_OR_ABORT("Unknown waypoint-class");
        }
    }
    std::map<OrderableFixedArray<CompressedScenePos, 3>, std::pair<size_t, WayPointLocation>> indices_street_wpts;
    for (const auto& [w0, w1] : street_way_point_edge_descriptors) {
        auto p0 = w0.position();
        auto p1 = w1.position();
        auto it0 = indices_street_wpts.insert({ OrderableFixedArray<CompressedScenePos, 3>{ p0 }, {indices_street_wpts.size(), WayPointLocation::NONE} });
        auto it1 = indices_street_wpts.insert({ OrderableFixedArray<CompressedScenePos, 3>{ p1 }, {indices_street_wpts.size(), WayPointLocation::NONE} });
        it0.first->second.second |= w0.location;
        it1.first->second.second |= w1.location;
    }
    way_points.points.resize(indices_terrain_wpts.size() + indices_street_wpts.size());
    std::set<size_t> grounded_way_points;
    for (const auto& [osm_id, adjacency_id] : indices_terrain_wpts) {
        const auto& node = nodes.at(osm_id);
        auto p2 = node.position;
        auto hwr = parse_height_with_reference(node.tags, "height", "height_reference", osm_id);
        if (hwr.has_value() && hwr.value().reference == HeightReference::WATER) {
            way_points.points[adjacency_id.first] = WayPoint{
                FixedArray<CompressedScenePos, 3>{p2(0), p2(1), (CompressedScenePos)(hwr.value().height * scale)},
                adjacency_id.second };
        } else {
            CompressedScenePos height;
            if (ground_bvh.height(height, p2)) {
                if (hwr.has_value()) {
                    if (hwr.value().reference != HeightReference::GROUND) {
                        THROW_OR_ABORT(osm_id + ": Unknown height reference, expected \"ground\"");
                    }
                    way_points.points[adjacency_id.first] = WayPoint{
                        FixedArray<CompressedScenePos, 3>{ p2(0), p2(1), height + (CompressedScenePos)(hwr.value().height * scale) },
                        adjacency_id.second
                    };
                } else {
                    way_points.points[adjacency_id.first] = WayPoint{
                        FixedArray<CompressedScenePos, 3>{ p2(0), p2(1), height },
                        adjacency_id.second
                    };
                    grounded_way_points.insert(adjacency_id.first);
                }
            } else {
                throw PointException<CompressedScenePos, 2>{ p2, osm_id + ": Could not determine height of original waypoint" };
            }
        }
    }
    for (const auto& [position, adjacency_id_offset] : indices_street_wpts) {
        auto point_id = indices_terrain_wpts.size() + adjacency_id_offset.first;
        way_points.points[point_id] = WayPoint{ position, adjacency_id_offset.second };
        grounded_way_points.insert(point_id);
    }
    way_points.adjacency = SparseArrayCcs<CompressedScenePos>{ArrayShape{
        indices_terrain_wpts.size() + indices_street_wpts.size(),
        indices_terrain_wpts.size() + indices_street_wpts.size()}};
    
    {
        auto insert_edge_1_lane = [&](const std::string& a, const std::string& b, const TerrainWayPoints& wps) {
            CompressedScenePos dist = (CompressedScenePos)std::sqrt(sum(squared(nodes.at(a).position - nodes.at(b).position)));
            if (!way_points.adjacency.column(indices_terrain_wpts.at(a).first).insert({indices_terrain_wpts.at(b).first, dist}).second) {
                THROW_OR_ABORT("Could not insert waypoint (0)");
            }
            if (wps.orientation == WayPointsOrientation::BIDIRECTIONAL) {
                if (!way_points.adjacency.column(indices_terrain_wpts.at(b).first).insert({indices_terrain_wpts.at(a).first, dist}).second) {
                    THROW_OR_ABORT("Could not insert waypoint (1)");
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
            if (!way_points.adjacency.column(i).insert({i, (CompressedScenePos)0.f}).second) {
                THROW_OR_ABORT("Could not insert waypoint (2)");
            }
        }
    }
    {
        for (const auto& e : street_way_point_edge_descriptors) {
            auto p0 = e.first.position();
            auto p1 = e.second.position();
            CompressedScenePos dist = (CompressedScenePos)std::sqrt(sum(squared(p0 - p1)));
            size_t col_id_0 = indices_terrain_wpts.size() + indices_street_wpts.at(OrderableFixedArray(p0)).first;
            size_t col_id_1 = indices_terrain_wpts.size() + indices_street_wpts.at(OrderableFixedArray(p1)).first;
            if (!way_points.adjacency.column(col_id_0).insert({ col_id_1, dist }).second) {
                THROW_OR_ABORT("Could not insert waypoint (3)");
            }
        }
        for (size_t i = 0; i < indices_street_wpts.size(); ++i) {
            if (!way_points.adjacency.column(indices_terrain_wpts.size() + i).insert({indices_terrain_wpts.size() + i, (CompressedScenePos)0.f}).second) {
                THROW_OR_ABORT("Could not insert waypoint (4)");
            }
        }
    }
    way_points.merge_neighbors(
        (CompressedScenePos)(merge_radius * scale),
        (CompressedScenePos)(error_radius * scale),
        [](auto& a, const auto& b) { a |= b; });
    auto interpolator = [&](
        const WayPoint& p0,
        const WayPoint& p1,
        double a0,
        double a1)
    {
        WayPoint res = p0 * a0 + p1 * a1;
        if (!ground_bvh.height3d(res.position(2), res.position)) {
            throw PointException<CompressedScenePos, 3>{ res.position, "Could not determine height of interpolated waypoint" };
        }
        return res;
    };
    if (!ssm != !to_meters) {
        THROW_OR_ABORT("Inconsistent to-meters mapping an navmesh parameters");
    }
    auto idef = interpolate_default<WayPoint>;
    InterpolatedIntermediatePointsCreator<WayPoint, decltype(idef)> default_iipc{
        waypoint_distance,
        idef };
    if (ssm != nullptr) {
        std::map<OrderableFixedArray<CompressedScenePos, 3>, dtPolyRef> poly_refs;
        for (auto&& [i, p] : enumerate(way_points.points)) {
            auto pm = dot1d(to_meters->model, funpack(p.position));
            if (!grounded_way_points.contains(i)) {
                p.position = pm.casted<CompressedScenePos>();
                continue;
            }
            auto lp = ssm->closest_point_on_navmesh(pm.casted<float>());
            if (!lp.has_value()) {
                throw PointException<CompressedScenePos, 3>{ p.position, "Could not find closest point on navmesh" };
            }
            if (!poly_refs.insert({ OrderableFixedArray(lp->position.casted<CompressedScenePos>()), lp->polyRef }).second) {
                // throw PointException<double, 3>{ p, "Found duplicate waypoint" };
                lwarn() << "Found duplicate waypoint after projection onto navmesh";
            }
            p.position = lp->position.casted<CompressedScenePos>();
        }
        way_points.update_adjacency();
        ShortestPathIntermediatePointsCreator spipc{ *ssm, poly_refs, (float)waypoint_distance };
        try {
            way_points.subdivide(
                [&](size_t r, size_t c, const CompressedScenePos& distance) -> std::vector<WayPoint> {
                    const auto& pR = way_points.points.at(r);
                    const auto& pC = way_points.points.at(c);
                    if (!grounded_way_points.contains(r) ||
                        !grounded_way_points.contains(c))
                    {
                        return default_iipc(pR, pC, distance);
                    } else {
                        auto positions = spipc(pR.position, pC.position);
                        std::vector<WayPoint> res;
                        res.reserve(positions.size());
                        for (const auto& p : positions) {
                            res.push_back({p, pR.flags | pC.flags});
                        }
                        return res;
                    }
                },
                SubdivisionType::ASYMMETRIC);
        } catch (const EdgeException<double>& e) {
            throw EdgeException<double>{dot1d(to_meters->view, e.a), dot1d(to_meters->view, e.b), e.what()};
        } catch (const PointException<double, 3>& e) {
            throw PointException<double, 3>{dot1d(to_meters->view, e.point), e.what()};
        }
        for (auto& p : way_points.points) {
            p.position = dot1d(to_meters->view, funpack(p.position)).casted<CompressedScenePos>();
            if (!any(p.flags & WayPointLocation::AIRWAY)) {
                if (!ground_bvh.height3d(p.position(2), p.position)) {
                    throw PointException<CompressedScenePos, 3>{ p.position, "Could not determine height of shortest-path waypoint" };
                }
            }
        }
        way_points.update_adjacency();
    } else {
        InterpolatedIntermediatePointsCreator<WayPoint, decltype(interpolator)> terrain_iipc{
            waypoint_distance * scale,
            interpolator };
        way_points.subdivide(
            [&](size_t r, size_t c, const CompressedScenePos& distance) -> std::vector<WayPoint> {
                if (!grounded_way_points.contains(r) ||
                    !grounded_way_points.contains(c))
                {
                    return default_iipc(way_points.points.at(r), way_points.points.at(c), distance);
                } else {
                    return terrain_iipc(way_points.points.at(r), way_points.points.at(c), distance);
                }
            },
            SubdivisionType::ASYMMETRIC);
    }
}
