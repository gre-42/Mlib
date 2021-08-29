#include "Indexed_Point_Set.hpp"
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

using namespace Mlib;

IndexedPointSet::IndexedPointSet()
: current_index_{0}
{}

IndexedPointSet::~IndexedPointSet()
{}

int IndexedPointSet::operator () (float x, float y) {
    OrderableFixedArray<float, 2> p{ x, y };
    if (auto it = pts_.find(p); it != pts_.end()) {
        return it->second;
    } else {
        auto pt = current_index_++;
        pts_.insert({p, pt});
        positions_.push_back(p(0));
        positions_.push_back(p(1));
        return pt;
    }
}

bool IndexedPointSet::exists(float x, float y) {
    OrderableFixedArray<float, 2> p{ x, y };
    return pts_.find(p) != pts_.end();
}

std::vector<double>& IndexedPointSet::positions() {
    return positions_;
}
