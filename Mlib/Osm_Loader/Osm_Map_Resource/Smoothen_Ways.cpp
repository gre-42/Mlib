#include "Smoothen_Ways.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Nodes_And_Ways.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Stats/Linspace.hpp>

using namespace Mlib;

FixedArray<double, 2> smooth_intermediate_node(
    const FixedArray<double, 2>& p0,
    const FixedArray<double, 2>& p1,
    const FixedArray<double, 2>& n0,
    const FixedArray<double, 2>& n1,
    double d,
    double t)
{
    // From: https://math.stackexchange.com/a/3253471/233679
    double a = 1 / squared(1 / t - 1);
    double t1 = (1 - 1 / (1 + a));

    return ((p0 + n0 * d * t) * (1 - t1) +
            (p1 - n1 * d * (1 - t)) * t1);
}

NodesAndWays Mlib::smoothen_ways(
    const NodesAndWays& naws,
    const std::set<std::string>& included_highways,
    float scale,
    float max_length)
{
    auto include_way = [&included_highways](const Way& way){
        return
            way.tags.contains("highway") &&
            included_highways.contains(way.tags.get("highway")) &&
            !way.tags.contains("smoothen", "no");
    };
    std::map<std::string, std::set<std::string>> node_neighbors;
    for (const auto& [way_id, way] : naws.ways) {
        if (!include_way(way)) {
            continue;
        }
        for (auto it = way.nd.begin(); it != way.nd.end(); ++it) {
            auto s = it;
            ++s;
            if (s != way.nd.end()) {
                node_neighbors[*it].insert(*s);
                node_neighbors[*s].insert(*it);
            }
        }
    }
    NodesAndWays result;
    result.nodes = naws.nodes;
    size_t segment_ctr = 0;
    for (const auto& [way_id, way] : naws.ways) {
        if (!include_way(way)) {
            result.ways[way_id] = way;
            continue;
        }
        std::list<std::string> new_nd;
        for (auto i0 = way.nd.begin(); i0 != way.nd.end(); ++i0) {
            new_nd.push_back(*i0);
            auto i1 = i0;
            ++i1;
            if (i1 == way.nd.end()) {
                break;
            }
            const auto& neighbors0 = node_neighbors.at(*i0);
            const auto& neighbors1 = node_neighbors.at(*i1);
            if ((neighbors0.size() > 2) || (neighbors1.size() > 2)) {
                continue;
            }
            if ((neighbors0.size() == 1) && (neighbors1.size() == 1)) {
                continue;
            }
            FixedArray<double, 2> n0;
            if (neighbors0.size() == 1) {
                n0 = naws.nodes.at(*i1).position - naws.nodes.at(*i0).position;
            } else {
                auto nrs = neighbors0;
                nrs.erase(*i1);
                n0 = naws.nodes.at(*i1).position - naws.nodes.at(*nrs.begin()).position;
            }
            n0 /= std::sqrt(sum(squared(n0)));
            FixedArray<double, 2> n1;
            if (neighbors1.size() == 1) {
                n1 = naws.nodes.at(*i1).position - naws.nodes.at(*i0).position;
            } else {
                auto nrs = neighbors1;
                nrs.erase(*i0);
                n1 = naws.nodes.at(*nrs.begin()).position - naws.nodes.at(*i0).position;
            }
            n1 /= std::sqrt(sum(squared(n1)));
            double d = std::sqrt(sum(squared(naws.nodes.at(*i1).position - naws.nodes.at(*i0).position))) / 2.;
            size_t n = 1 + size_t(d / scale / max_length);
            if (n <= 2) {
                continue;
            }
            auto t = Linspace<double>(0., 1., n);
            for (size_t i = 1; i < n - 1; ++i) {
                auto snode_id = "snode_" + std::to_string(segment_ctr++);
                auto snode_p = smooth_intermediate_node(
                    naws.nodes.at(*i0).position,
                    naws.nodes.at(*i1).position,
                    n0,
                    n1,
                    d,
                    t[i]);
                new_nd.push_back(snode_id);
                result.nodes[snode_id] = Node{.position = snode_p, .tags = {}};
            }
        }
        result.ways[way_id] = Way{
            .nd = new_nd,
            .tags = way.tags};
    }
    return result;
}
