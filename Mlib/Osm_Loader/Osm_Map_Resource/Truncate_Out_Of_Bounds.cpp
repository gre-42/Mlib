#include "Truncate_Out_Of_Bounds.hpp"
#include <Mlib/Geometry/Primitives/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Elements.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Nodes_And_Ways.hpp>
#include <algorithm>

using namespace Mlib;

auto chunks_fulfilling_predicate(const auto& sequence, const auto& predicate) {
    return sequence 
        | std::views::chunk_by([&](auto&& a, auto&& b) { 
            return predicate(a) == predicate(b); 
        })
        | std::views::filter([&](auto&& chunk) { 
            return predicate(*chunk.begin()); 
        });
}

void Mlib::truncate_out_of_bounds(
    NodesAndWays& naws,
    const AxisAlignedBoundingBox<CompressedScenePos, 2>& aabb)
{
    Map<std::string, Way> new_ways;
    auto node_is_outside = [&](const auto& node){ return !aabb.contains(node.second.position); };
    auto nd_exists = [&](const auto& nd){ return naws.nodes.contains(nd); };
    std::erase_if(naws.nodes, node_is_outside);
    std::erase_if(naws.ways, [&](const auto& way){
        if (std::ranges::all_of(way.second.nd, nd_exists)) {
            return false;
        }
        if (way.second.nd.empty()) {
            throw std::runtime_error("Way is empty");
        }
        if (way.second.nd.front() == way.second.nd.back()) {
            return true;
        }
        for (const auto& [i, w1] : enumerate(chunks_fulfilling_predicate(way.second.nd, nd_exists))) {
            auto nd = std::list(w1.begin(), w1.end());
            if (nd.empty()) {
                throw std::runtime_error("Way chunk is empty");
            }
            if (nd.size() == 1) {
                continue;
            }
            new_ways.add(std::format("split_{}_{}", i, way.first), Way{std::move(nd), way.second.tags});
        }
        return true;
    });
    naws.ways.merge(new_ways);
    // for (const auto& w : naws.ways) {
    //     for (const auto& nd : w.second.nd) {
    //         if (!naws.nodes.contains(nd)) {
    //             throw std::runtime_error("Node with name \"" + nd + "\" is missing");
    //         }
    //     }
    // }
}
