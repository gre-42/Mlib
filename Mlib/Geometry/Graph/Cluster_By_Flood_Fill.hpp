#pragma once
#include <list>
#include <unordered_map>

namespace Mlib {

template <class TNodes, class TIsNeighbor>
std::list<std::list<typename TNodes::value_type*>> cluster_by_flood_fill(
    TNodes& nodes,
    const TIsNeighbor& is_neighbor)
{
    using value_type = TNodes::value_type;
    using Neighbors = std::unordered_map<value_type*, std::list<value_type*>>;
    Neighbors neighbors;
    for (auto n0 = nodes.begin(); n0 != nodes.end(); ++n0) {
        auto& neighbors0 = neighbors[&*n0];
        auto n1 = n0;
        ++n1;
        while (n1 != nodes.end()) {
            if (is_neighbor(*n0, *n1)) {
                auto& neighbors1 = neighbors[&*n1];
                // linfo() << *n0 << " - " << *n1;
                neighbors0.push_back(&*n1);
                neighbors1.push_back(&*n0);
            }
            ++n1;
        }
    }
    std::list<std::list<value_type*>> result;
    Neighbors active_nodes;
    while (!neighbors.empty()) {
        if (active_nodes.empty()) {
            active_nodes.insert(neighbors.extract(neighbors.begin()));
            // linfo() << "take first " << *active_nodes.begin()->first;
            result.emplace_back();
        }
        while (!active_nodes.empty()) {
            auto active = active_nodes.extract(active_nodes.begin());
            // linfo() << "add " << *active.key();
            result.back().push_back(active.key());
            for (auto& n : active.mapped()) {
                // linfo() << "visit " << *n;
                auto nit = neighbors.find(n);
                if (nit != neighbors.end()) {
                    // linfo() << "activate " << *nit->first;
                    active_nodes.insert(neighbors.extract(nit));
                }
            }
        }
    }
    return result;
}

}
