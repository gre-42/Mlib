#include "Indent_Buildings.hpp"
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

static const std::string INDENT_INFIX = "_indented_";

NodesAndWays Mlib::indent_buildings(
    const NodesAndWays& naws,
    CompressedScenePos amount)
{
    NodesAndWays result {
        .nodes = naws.nodes,
        .ways = {}
    };
    for (const auto& [way_id, way] : naws.ways) {
        if (![&](){
            if (way.tags.contains("layer") &&
                (safe_stoi(way.tags.at("layer")) != 0)) {
                return false;
            }
            if (way.tags.find("building") == way.tags.end()) {
                return false;
            }
            if (excluded_buildings.contains(way.tags.at("building"))) {
                return false;
            }
            if ((way.nd.size() < 2) || (way.nd.front() != way.nd.back())) {
                THROW_OR_ABORT("Building contour not closed: \"" + way_id + '"');
            }
            if (amount == (CompressedScenePos)0.f) {
                return false;
            }
            return true;
        }())
        {
            result.ways.add(way_id, way);
            continue;
        }
        auto way_nd = way.nd;
        // Note that this is clockwise, while the terrain regions use ccw.
        if (compute_area_clockwise(way.nd, naws.nodes, 1.) < 0) {
            way_nd.reverse();
        }
        Map<std::string, Node> indented_nodes;
        std::list<std::string> indented_way_nd;
        std::vector<FixedArray<CompressedScenePos, 2>> indented_contour;
        std::vector<FixedArray<CompressedScenePos, 2>> original_contour;
        for (auto a = way_nd.begin(); ; ++a) {
            if (&*a == &way_nd.back()) {
                auto indented_node_id = way_id + INDENT_INFIX + *(++way_nd.begin());
                indented_way_nd.emplace_back(indented_node_id);
                break;
            }
            auto b = a;
            ++b;
            assert_true(b != way_nd.end());
            if (&*b == &way_nd.back()) {
                b = way_nd.begin();
            }
            auto c = b;
            ++c;
            assert_true(c != way_nd.end());
            if (&*c == &way_nd.back()) {
                c = way_nd.begin();
            }
            OsmRectangle2D rect = uninitialized;
            if (!OsmRectangle2D::from_line(
                    rect,
                    naws.nodes.at(*a).position,
                    naws.nodes.at(*a).position,
                    naws.nodes.at(*b).position,
                    naws.nodes.at(*c).position,
                    naws.nodes.at(*b).position,
                    naws.nodes.at(*b).position,
                    2.f * amount,
                    2.f * amount,
                    2.f * amount,
                    2.f * amount,
                    2.f * amount,
                    2.f * amount))
            {
                THROW_OR_ABORT("Error computing building indentation for node \"" + *b + '"');
            }
            auto indented_node_id = way_id + INDENT_INFIX + *b;
            indented_way_nd.emplace_back(indented_node_id);

            indented_contour.push_back(rect.p01_);
            original_contour.push_back(naws.nodes.at(*b).position);
            indented_nodes.add(indented_node_id, rect.p01_, naws.nodes.at(*b).tags);
        }
        if (indented_contour.size() != way.nd.size() - 1) {
            THROW_OR_ABORT("Error indenting building \"" + way_id + '"');
        }
        // plot_mesh_svg(
        //     "/tmp/building_" + way_id + ".svg",
        //     600.,
        //     600.,
        //     {},
        //     {},
        //     { indented_contour, original_contour },
        //     {},
        //     (CompressedScenePos)2e-1f);
        if (!contour_is_ok(indented_contour, MIN_VERTEX_DISTANCE)) {
            result.ways.add(way_id, way);
        } else {
            result.nodes.merge(indented_nodes);
            if (!indented_nodes.empty()) {
                THROW_OR_ABORT("Could not add indented nodes");
            }
            result.ways.add(way_id, std::move(indented_way_nd), way.tags);

            Map<std::string, std::string> grass_tags;
            grass_tags.add("terrain_region", "grass");
            result.ways.add(way_id + "_grass", way.nd, std::move(grass_tags));
        }
    }
    return result;
}
