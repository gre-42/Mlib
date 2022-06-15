#include "P2t_Point_Set.hpp"
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <poly2tri/common/shapes.h>

using namespace Mlib;

P2tPointSet::P2tPointSet(const std::list<FixedArray<double, 2>>& steiner_points) {
    for (const auto& p : steiner_points) {
        // Ignore result
        steiner_pts_.insert({
            OrderableFixedArray<double, 2>{p},
            std::make_unique<p2t::Point>(p(0), p(1))});
    }
}

P2tPointSet::~P2tPointSet()
{}

p2t::Point* P2tPointSet::operator () (double x, double y) {
    auto p = OrderableFixedArray<double, 2>{x, y};
    if (auto it = pts_.find(p); it != pts_.end()) {
        return it->second.get();
    }
    if (auto it = steiner_pts_.find(p); it != steiner_pts_.end()) {
        auto pt = it->second.get();
        pts_.insert({p, std::move(it->second)});
        steiner_pts_.erase(it);
        return pt;
    }
    auto pt = std::make_unique<p2t::Point>(x, y);
    p2t::Point* ppt = pt.get();
    pts_.insert({p, std::move(pt)});
    return ppt;
}

std::vector<p2t::Point*> P2tPointSet::remaining_steiner_points() {
    std::vector<p2t::Point*> result;
    result.reserve(steiner_pts_.size());
    for (auto& p : steiner_pts_) {
        result.push_back(p.second.get());
    }
    return result;
}
