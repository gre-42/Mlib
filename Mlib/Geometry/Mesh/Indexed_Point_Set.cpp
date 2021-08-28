#include "Indexed_Point_Set.hpp"
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

using namespace Mlib;

IndexedPointSet::IndexedPointSet(const std::list<FixedArray<float, 2>>& steiner_points)
: current_index_{0}
{
    for (const auto& p : steiner_points) {
        // Ignore result
        pts_.insert({
            OrderableFixedArray<float, 2>{p},
            current_index_++});
        positions_.push_back(p(0));
        positions_.push_back(p(1));
    }
}

IndexedPointSet::~IndexedPointSet()
{}

int IndexedPointSet::operator () (float x, float y) {
    auto p = OrderableFixedArray<float, 2>{x, y};
    if (auto it = pts_.find(p); it != pts_.end()) {
        return it->second;
    }
    auto pt = current_index_++;
    pts_.insert({p, pt});
    positions_.push_back(p(0));
    positions_.push_back(p(1));
    return pt;
}

std::vector<double>& IndexedPointSet::positions() {
    return positions_;
}
