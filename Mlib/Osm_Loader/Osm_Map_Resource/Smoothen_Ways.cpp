#include "Smoothen_Ways.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Sigmoid.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Get_Way_Width.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Nodes_And_Ways.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

static FixedArray<CompressedScenePos, 2> smooth_intermediate_node(
    const FixedArray<CompressedScenePos, 2>& p0,
    const FixedArray<CompressedScenePos, 2>& p1,
    const FixedArray<double, 2>& n0,
    const FixedArray<double, 2>& n1,
    double d,
    double t)
{
    double t1 = sigmoid(t);

    return ((funpack(p0) + n0 * d * t) * (1 - t1) +
            (funpack(p1) - n1 * d * (1 - t)) * t1).casted<CompressedScenePos>();
}

class IncludeWay {
public:
    explicit IncludeWay(
        const std::set<std::string>& included_highways,
        const std::set<std::string>& included_aeroways,
        const Way& way)
    {
        if (way.tags.contains("smoothen", "no")) {
            force_include_ = false;
            include_way_ = false;
            include_some_nodes_ = false;
        } else {
            force_include_ = way.tags.contains("smoothen", "yes");
            include_way_ =
                force_include_ ||
                (way.tags.contains("highway") &&
                    included_highways.contains(way.tags.get("highway"))) ||
                (way.tags.contains("aeroway") &&
                    included_aeroways.contains(way.tags.get("aeroway")));
            include_some_nodes_ =
                include_way_ ||
                way.tags.contains("terrain_region") ||
                way.tags.contains("barrier");
        }
    }
    bool include_some() const {
        return include_some_nodes_;
    }
    bool include(const Node& a, const Node& b) const {
        return include(a, include_way_) && include(b, include_way_);
    }
    bool force_include(const Node& a, const Node& b) const {
        return include(a, force_include_) && include(b, force_include_);
    }
private:
    static bool include(const Node& node, bool deflt) {
        auto v = node.tags.try_get("smoothen");
        if (v == nullptr) {
            return deflt;
        }
        if (*v == "yes") {
            return true;
        }
        if (*v == "no") {
            return false;
        }
        THROW_OR_ABORT("Unsupported smoothing mode");
    }
    bool force_include_;
    bool include_way_;
    bool include_some_nodes_;
};

NodesAndWays Mlib::smoothen_ways(
    const NodesAndWays& naws,
    const std::set<std::string>& included_highways,
    const std::set<std::string>& included_aeroways,
    float default_street_width,
    float default_lane_width,
    float scale,
    float max_length)
{
    std::map<std::string, std::set<std::string>> node_neighbors;
    std::map<std::string, std::set<std::string>> node_ways;
    for (const auto& [way_id, way] : naws.ways) {
        IncludeWay iw{ included_highways, included_aeroways, way };
        if (!iw.include_some()) {
            continue;
        }
        for (auto it = way.nd.begin(); it != way.nd.end(); ++it) {
            auto s = it;
            ++s;
            if (s == way.nd.end()) {
                break;
            }
            if (!iw.include(naws.nodes.at(*s), naws.nodes.at(*it))) {
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
        IncludeWay iw{ included_highways, included_aeroways, way };
        if (!iw.include_some()) {
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
            if (!iw.include(nd0, nd1)) {
                continue;
            }
            const auto& neighbors0 = node_neighbors.at(*i0);
            const auto& neighbors1 = node_neighbors.at(*i1);
            if ((neighbors0.size() > 2) || (neighbors1.size() > 2)) {
                continue;
            }
            if ((neighbors0.size() == 1) &&
                (neighbors1.size() == 1) &&
                (!iw.force_include(nd0, nd1))) {
                continue;
            }
            auto models_and_widths_identical = [&](const std::string& i) {
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
                    THROW_OR_ABORT(
                        "Number of ways neither 1 or 2 despite number of neighbors check at node \"" + i +
                        "\". Neighbors: " + Mlib::join(", ", node_neighbors.at(i)) +
                        ". Ways: " + Mlib::join(", ", iways));
                }
            };
            if (!models_and_widths_identical(*i0) || !models_and_widths_identical(*i1)) {
                continue;
            }
            auto n_line = funpack(nd1.position - nd0.position);
            double line_len = std::sqrt(sum(squared(n_line)));
            n_line /= line_len;
            FixedArray<double, 2> n0 = uninitialized;
            if (neighbors0.size() == 1) {
                n0 = n_line;
            } else {
                auto nrs = neighbors0;
                nrs.erase(*i1);
                auto n0_1 = funpack(nd0.position - naws.nodes.at(*nrs.begin()).position);
                n0_1 /= std::sqrt(sum(squared(n0_1)));
                n0 = n_line + n0_1;
                n0 /= std::sqrt(sum(squared(n0)));
            }
            FixedArray<double, 2> n1 = uninitialized;
            if (neighbors1.size() == 1) {
                n1 = n_line;
            } else {
                auto nrs = neighbors1;
                nrs.erase(*i0);
                auto n1_0 = funpack(naws.nodes.at(*nrs.begin()).position - nd1.position);
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
                result.nodes.add(snode_id, Node{.position = snode_p, .tags = tags});
            }
        }
        result.ways[way_id] = Way{
            .nd = new_nd,
            .tags = way.tags};
    }
    return result;
}
