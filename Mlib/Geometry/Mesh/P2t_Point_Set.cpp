#include "P2t_Point_Set.hpp"
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <poly2tri/common/shapes.h>

using namespace Mlib;

P2tPointSet::P2tPointSet(
    const std::list<FixedArray<CompressedScenePos, 2>>& steiner_points,
    double scale)
    : scale_{ scale }
    , coords_{ "coords", [](const auto& p) { return "point address"; } }
{
    for (const auto& p : steiner_points) {
        // Ignore result
        auto it = steiner_pts_.try_emplace(
            OrderableFixedArray<CompressedScenePos, 2>{p},
            gen_point(p(0), p(1)));
        if (it.second) {
            coords_.add(it.first->second.get(), p);
        }
    }
}

P2tPointSet::~P2tPointSet()
{}

std::unique_ptr<p2t::Point> P2tPointSet::gen_point(
    CompressedScenePos x,
    CompressedScenePos y) const
{
    return std::make_unique<p2t::Point>(funpack(x) * scale_, funpack(y) * scale_);
}

p2t::Point* P2tPointSet::operator () (CompressedScenePos x, CompressedScenePos y) {
    auto p = OrderableFixedArray<CompressedScenePos, 2>{ x, y };
    if (auto it = pts_.find(p); it != pts_.end()) {
        return it->second.get();
    }
    if (auto it = steiner_pts_.find(p); it != steiner_pts_.end()) {
        auto pt = it->second.get();
        pts_.add(p, std::move(it->second));
        steiner_pts_.erase(it);
        return pt;
    }
    auto pt = gen_point(x, y);
    p2t::Point* ppt = pt.get();
    pts_.add(p, std::move(pt));
    coords_.add(ppt, x, y);
    return ppt;
}

std::vector<p2t::Point*> P2tPointSet::remaining_steiner_points() const {
    std::vector<p2t::Point*> result;
    result.reserve(steiner_pts_.size());
    for (auto& p : steiner_pts_) {
        result.push_back(p.second.get());
    }
    return result;
}

const FixedArray<CompressedScenePos, 2>& P2tPointSet::coords(const p2t::Point* p) const {
    return coords_.get(p);
}
