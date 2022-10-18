#include "Smoothen_Ways.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Get_Way_Width.hpp>
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
    float default_street_width,
    float default_lane_width,
    float scale,
    float max_length)
{
    auto include_some_nodes = [](const Way& way) {
        return way.tags.contains("terrain_region") ||
               way.tags.contains("barrier");
    };
    auto include_all_nodes = [&included_highways](const Way& way){
        return
            way.tags.contains("highway") &&
            included_highways.contains(way.tags.get("highway")) &&
            !way.tags.contains("smoothen", "no");
    };
    auto include_node = [](const Node& node) {
        return node.tags.contains("smoothen", "yes");
    };
    std::map<std::string, std::set<std::string>> node_neighbors;
    std::map<std::string, std::set<std::string>> node_ways;
    for (const auto& [way_id, way] : naws.ways) {
        bool include_all = include_all_nodes(way);
        if (!include_all && !include_some_nodes(way)) {
            continue;
        }
        for (auto it = way.nd.begin(); it != way.nd.end(); ++it) {
            auto s = it;
            ++s;
            if (s == way.nd.end()) {
                break;
            }
            if (!include_all && (!include_node(naws.nodes.at(*s)) || !include_node(naws.nodes.at(*it)))) {
                continue;
            }
            node_neighbors[*it].insert(*s);
            node_neighbors[*s].insert(*it);
            node_ways[*it].insert(way_id);
            node_ways[*s].insert(way_id);
        }
    }
    NodesAndWays result;
    result.nodes = naws.nodes;
    size_t segment_ctr = 0;
    for (const auto& [way_id, way] : naws.ways) {
        bool include_all = include_all_nodes(way);
        if (!include_all && !include_some_nodes(way)) {
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
            const auto& nd0 = naws.nodes.at(*i0);
            const auto& nd1 = naws.nodes.at(*i1);
            if (!include_all && (!include_node(nd0) || !include_node(nd1))) {
                continue;
            }
            const auto& neighbors0 = node_neighbors.at(*i0);
            const auto& neighbors1 = node_neighbors.at(*i1);
            if ((neighbors0.size() > 2) || (neighbors1.size() > 2)) {
                continue;
            }
            if (include_all && ((neighbors0.size() == 1) && (neighbors1.size() == 1))) {
                continue;
            }
            auto models_and_widths_identical = [&node_ways, &naws, &default_street_width, &default_lane_width](const std::string& i) {
                const auto& iways = node_ways.at(i);
                if (iways.size() == 1) {
                    return true;
                }
                if (iways.size() == 2) {
                    const auto& tags0 = naws.ways.at(*iways.begin()).tags;
                    const auto& tags1 = naws.ways.at(*++iways.begin()).tags;
                    if (get_way_width(tags0, default_street_width, default_lane_width) !=
                        get_way_width(tags1, default_street_width, default_lane_width))
                    {
                        return false;
                    }
                    auto model0 = tags0.find("model");
                    auto model1 = tags1.find("model");
                    if ((model0 == tags0.end()) && (model1 == tags1.end())) {
                        return true;
                    }
                    if ((model0 != tags0.end()) && (model1 != tags1.end())) {
                        return model0->second == model1->second;
                    }
                    return false;
                } else {
                    throw std::runtime_error("Number of ways neither 1 or 2 despite number of neighbors check");
                }
            };
            if (!models_and_widths_identical(*i0) || !models_and_widths_identical(*i1)) {
                continue;
            }
            auto n_line = nd1.position - nd0.position;
            double line_len = std::sqrt(sum(squared(n_line)));
            n_line /= line_len;
            FixedArray<double, 2> n0;
            if (neighbors0.size() == 1) {
                n0 = n_line;
            } else {
                auto nrs = neighbors0;
                nrs.erase(*i1);
                auto n0_1 = nd0.position - naws.nodes.at(*nrs.begin()).position;
                n0_1 /= std::sqrt(sum(squared(n0_1)));
                n0 = n_line + n0_1;
                n0 /= std::sqrt(sum(squared(n0)));
            }
            FixedArray<double, 2> n1;
            if (neighbors1.size() == 1) {
                n1 = n_line;
            } else {
                auto nrs = neighbors1;
                nrs.erase(*i0);
                auto n1_0 = naws.nodes.at(*nrs.begin()).position - nd1.position;
                n1_0 /= std::sqrt(sum(squared(n1_0)));
                n1 = n_line + n1_0;
                n1 /= std::sqrt(sum(squared(n1)));
            }
            size_t n = 1 + size_t(line_len / scale / max_length);
            if (n <= 2) {
                continue;
            }
            double d = line_len / 2.;
            auto t = Linspace<double>(0., 1., n);
            for (size_t i = 1; i < n - 1; ++i) {
                auto snode_id = "snode_" + std::to_string(segment_ctr++);
                auto snode_p = smooth_intermediate_node(
                    nd0.position,
                    nd1.position,
                    n0,
                    n1,
                    d,
                    t[i]);
                new_nd.push_back(snode_id);
                auto tags = nd0.tags;
                tags.insert(nd1.tags.begin(), nd1.tags.end());
                result.nodes[snode_id] = Node{.position = snode_p, .tags = tags};
            }
        }
        result.ways[way_id] = Way{
            .nd = new_nd,
            .tags = way.tags};
    }
    return result;
}
