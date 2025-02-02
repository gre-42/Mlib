#include "Report_Osm_Problems.hpp"
#include <Mlib/Geometry/Mesh/Contour.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Compute_Area.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

void Mlib::report_osm_problems(
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways)
{
    std::set<std::pair<std::string, std::string>> edges;
    std::map<std::string, unsigned int> node_ctr;
    for (const auto& w : ways) {
        const auto& tags = w.second.tags;
        if (tags.find("building") == tags.end()) {
            continue;
        }
        if (tags.contains("layer") &&
            (safe_stoi(tags.at("layer")) != 0))
        {
            continue;
        }
        bool area_cw = (compute_area_clockwise(w.second.nd, nodes, 1.) > 0.);
        std::string n_old;
        for (const auto& n : w.second.nd) {
            if (n.empty()) {
                THROW_OR_ABORT("Node id is empty");
            }
            if (!n_old.empty()) {
                auto edge = std::make_pair(n_old, n);
                auto iedge = std::make_pair(n, n_old);
                if (area_cw) {
                    std::swap(edge.first, edge.second);
                    std::swap(iedge.first, iedge.second);
                }
                if (edges.contains(iedge)) {
                    edges.erase(iedge);
                } else {
                    edges.insert(edge);
                }
            }
            n_old = n;
        }
    }
    for (const auto& e : edges) {
        ++node_ctr[e.first];
        ++node_ctr[e.second];
    }
    for (const auto& c : node_ctr) {
        if (c.second > 2) {
            lerr() << "To modify: " << c.first << " " << c.second;
        }
    }
}
