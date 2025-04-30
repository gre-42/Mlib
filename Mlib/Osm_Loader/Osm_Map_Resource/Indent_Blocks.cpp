#include "Indent_Blocks.hpp"
#include <Mlib/Geometry/Mesh/Plot.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Compute_Area.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Contour_Is_Ok.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Nodes_And_Ways.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Limits.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Rectangle_2D.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>
#include <set>

using namespace Mlib;

NodesAndWays Mlib::indent_blocks(
    const NodesAndWays& naws,
    CompressedScenePos amount,
    ConnectorIndentation connector_indentation)
{
    std::map<std::string, std::set<std::string>> node_successors;
    std::map<std::string, std::set<std::string>> node_predecessors;
    {
        std::set<std::pair<std::string, std::string>> edges;
        for (const auto& [way_id, way] : naws.ways) {
            if (way.tags.contains("layer") &&
                (safe_stoi(way.tags.at("layer")) != 0)) {
                continue;
            }
            if (way.tags.find("building") == way.tags.end()) {
                continue;
            }
            if (excluded_buildings.contains(way.tags.at("building"))) {
                continue;
            }
            if ((way.nd.size() < 2) || (way.nd.front() != way.nd.back())) {
                THROW_OR_ABORT("Building contour not closed: \"" + way_id + '"');
            }
            auto way_nd = way.nd;
            // Note that this is clockwise, while the terrain regions use ccw.
            if (compute_area_clockwise(way.nd, naws.nodes, 1.) < 0) {
                way_nd.reverse();
            }
            for (auto it = way_nd.begin(); it != way_nd.end(); ++it) {
                auto s = it;
                ++s;
                if (s == way_nd.end()) {
                    break;
                }
                node_successors[*it].insert(*s);
                node_predecessors[*s].insert(*it);
                edges.insert({ *it, *s });
            }
        }
        for (const auto& edge : edges) {
            if (edges.contains({edge.second, edge.first})) {
                if (connector_indentation == ConnectorIndentation::MOVING) {
                    if (auto it = node_successors.find(edge.first); it != node_successors.end()) {
                        it->second.erase(edge.second);
                    }
                    if (auto it = node_predecessors.find(edge.first); it != node_predecessors.end()) {
                        it->second.erase(edge.second);
                    }
                } else if (connector_indentation == ConnectorIndentation::FIXED) {
                    node_successors.erase(edge.first);
                    node_predecessors.erase(edge.first);
                } else {
                    THROW_OR_ABORT("Unknown connector indentation");
                }
            }
        }
    }
    NodesAndWays result {
        .nodes = {},
        .ways = naws.ways
    };
    for (auto& [way_id, way] : result.ways) {
        Map<std::string, Node> indented_nodes;
        Map<std::string, Node> original_nodes;
        std::vector<FixedArray<CompressedScenePos, 2>> indented_contour;
        indented_contour.reserve(way.nd.size());
        for (const auto& node_id : way.nd) {
            const auto& node = naws.nodes.at(node_id);
            const auto& a = node_predecessors.find(node_id);
            const auto& c = node_successors.find(node_id);
            if ((a == node_predecessors.end()) || (a->second.size() != 1) ||
                (c == node_successors.end()) || (c->second.size() != 1))
            {
                result.nodes.try_emplace(node_id, node.position, node.tags);
                if (&node_id != &way.nd.back()) {
                    indented_contour.push_back(node.position);
                }
                continue;
            }
            if (&node_id == &way.nd.back()) {
                continue;
            }
            OsmRectangle2D rect = uninitialized;
            if (!OsmRectangle2D::from_line(
                    rect,
                    naws.nodes.at(*a->second.begin()).position,
                    naws.nodes.at(*a->second.begin()).position,
                    node.position,
                    naws.nodes.at(*c->second.begin()).position,
                    node.position,
                    node.position,
                    2.f * amount,
                    2.f * amount,
                    2.f * amount,
                    2.f * amount,
                    2.f * amount,
                    2.f * amount))
            {
                THROW_OR_ABORT("Error computing building indentation for node \"" + node_id + '"');
            }
            indented_contour.push_back(rect.p01_);
            indented_nodes.add(node_id, rect.p01_, node.tags);
            original_nodes.add(node_id, node);
        }
        if (!indented_nodes.empty()) {
            // plot_mesh_svg(
            //     "/tmp/building_" + way_id + ".svg",
            //     600.,
            //     600.,
            //     {},
            //     {},
            //     {indented_contour},
            //     {},
            //     (CompressedScenePos)0.5f);
            if (indented_contour.size() != way.nd.size() - 1) {
                THROW_OR_ABORT("Error indenting building \"" + way_id + '"');
            }
            if (!contour_is_ok(indented_contour, MIN_VERTEX_DISTANCE)) {
                result.nodes.merge(original_nodes);
            } else {
                result.nodes.merge(indented_nodes);
                way.tags["indentation"] = std::to_string(amount);
            }
        }
    }
    return result;
}
