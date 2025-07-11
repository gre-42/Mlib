#pragma once
#include <Mlib/Geometry/Graph/Points_And_Adjacency.hpp>
#include <algorithm>

namespace Mlib {

template <class TPoint>
void shortest_path_multiple_targets(
    const PointsAndAdjacency<TPoint>& points_and_adjacency,
    const std::vector<size_t>& targets,
    std::vector<size_t>& predecessors,
    std::vector<typename TPoint::value_type>& total_distances)
{
    using TData = typename TPoint::value_type;
    predecessors = std::vector<size_t>(points_and_adjacency.points.size(), SIZE_MAX);
    total_distances = std::vector<TData>(points_and_adjacency.points.size(), std::numeric_limits<TData>::max());
    for (size_t i : targets) {
        total_distances[i] = (TData)0.f;
    }
    std::vector<size_t> active_nodes = targets;
    auto cmp = [&total_distances](size_t a, size_t b){return total_distances[a] < total_distances[b];};
    while (!active_nodes.empty()) {
        std::make_heap(active_nodes.begin(), active_nodes.end(), cmp);
        std::pop_heap(active_nodes.begin(), active_nodes.end(), cmp);
        size_t i = active_nodes.back();
        active_nodes.pop_back();
        for (const auto& [n, d] : points_and_adjacency.adjacency.column(i)) {
            if (total_distances[i] + d < total_distances[n]) {
                total_distances[n] = total_distances[i] + d;
                predecessors[n] = i;
                active_nodes.push_back(n);
                std::push_heap(active_nodes.begin(), active_nodes.end(), cmp);
            }
        }
    }
}

}
