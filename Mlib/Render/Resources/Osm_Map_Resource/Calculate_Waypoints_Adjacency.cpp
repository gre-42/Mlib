#include "Calculate_Waypoint_Adjacency.hpp"
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>

using namespace Mlib;

void Mlib::calculate_waypoint_adjacency(
    PointsAndAdjacency<float, 2>& way_points,
    const std::list<Building>& way_point_lines,
    const std::list<std::pair<std::string, std::string>>& way_point_edges_1_lane,
    const std::list<std::pair<FixedArray<float, 3>, FixedArray<float, 3>>>& way_point_edges_2_lanes,
    const std::map<std::string, Node>& nodes)
{
    std::map<std::string, size_t> indices_1_lane;
    for (const Building& bu : way_point_lines) {
        for (const std::string& n : bu.way.nd) {
            indices_1_lane.insert({n, indices_1_lane.size()});
        }
    }
    for (const auto& e : way_point_edges_1_lane) {
        indices_1_lane.insert({e.first, indices_1_lane.size()});
        indices_1_lane.insert({e.second, indices_1_lane.size()});
    }
    std::map<OrderableFixedArray<float, 2>, size_t> indices_2_lanes;
    for (const auto& e : way_point_edges_2_lanes) {
        indices_2_lanes.insert({OrderableFixedArray<float, 2>{e.first(0), e.first(1)}, indices_2_lanes.size()});
        indices_2_lanes.insert({OrderableFixedArray<float, 2>{e.second(0), e.second(1)}, indices_2_lanes.size()});
    }
    way_points.points.resize(indices_1_lane.size() + indices_2_lanes.size());
    for (const auto& p : indices_1_lane) {
        way_points.points[p.second] = nodes.at(p.first).position;
    }
    for (const auto& p : indices_2_lanes) {
        way_points.points[indices_1_lane.size() + p.second] = p.first;
    }
    way_points.adjacency = SparseArrayCcs<float>{ArrayShape{
        indices_1_lane.size() + indices_2_lanes.size(),
        indices_1_lane.size() + indices_2_lanes.size()}};
    
    {
        auto insert_edge_1_lane = [&way_points, &nodes, &indices_1_lane](const std::string& a, const std::string& b) {
            float dist = std::sqrt(sum(squared(nodes.at(a).position - nodes.at(b).position)));
            if (!way_points.adjacency.column(indices_1_lane.at(a)).insert({indices_1_lane.at(b), dist}).second) {
                throw std::runtime_error("Could not insert waypoint (0)");
            }
            if (!way_points.adjacency.column(indices_1_lane.at(b)).insert({indices_1_lane.at(a), dist}).second) {
                throw std::runtime_error("Could not insert waypoint (1)");
            }
        };
        for (const Building& bu : way_point_lines) {
            for (auto it = bu.way.nd.begin(); it != bu.way.nd.end(); ++it) {
                auto s = it;
                ++s;
                if (s != bu.way.nd.end()) {
                    insert_edge_1_lane(*s, *it);
                }
            }
        }
        for (const auto& e : way_point_edges_1_lane) {
            insert_edge_1_lane(e.first, e.second);
        }
        for (size_t i = 0; i < indices_1_lane.size(); ++i) {
            if (!way_points.adjacency.column(i).insert({i, 0.f}).second) {
                throw std::runtime_error("Could not insert waypoint (2)");
            }
        }
    }
    {
        for (const auto& e : way_point_edges_2_lanes) {
            float dist = std::sqrt(sum(squared(e.first - e.second)));
            size_t col_id_0 = indices_1_lane.size() + indices_2_lanes.at(OrderableFixedArray<float, 2>{e.first(0), e.first(1)});
            size_t col_id_1 = indices_1_lane.size() + indices_2_lanes.at(OrderableFixedArray<float, 2>{e.second(0), e.second(1)});
            if (!way_points.adjacency.column(col_id_0).insert({col_id_1, dist}).second) {
                throw std::runtime_error("Could not insert waypoint (3)");
            }
        }
        for (size_t i = 0; i < indices_2_lanes.size(); ++i) {
            if (!way_points.adjacency.column(indices_1_lane.size() + i).insert({indices_1_lane.size() + i, 0.f}).second) {
                throw std::runtime_error("Could not insert waypoint (4)");
            }
        }
    }
}
