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
            gen_point(p));
        if (it.second) {
            coords_.add(it.first->second.get(), p);
        }
    }
}

P2tPointSet::~P2tPointSet()
{}

std::unique_ptr<p2t::Point> P2tPointSet::gen_point(
    const FixedArray<CompressedScenePos, 2>& c) const
{
    auto sc = funpack(c) * scale_;
    auto res = std::make_unique<p2t::Point>(sc(0), sc(1));
    auto rc = compute_coords(res.get());
    if (any(rc != c)) {
        THROW_OR_ABORT((std::stringstream() << "Cannot map point: " << c << " != " << rc).str());
    }
    return res;
}

p2t::Point* P2tPointSet::operator () (const FixedArray<CompressedScenePos, 2>& c) {
    auto p = OrderableFixedArray{ c };
    if (auto it = pts_.find(p); it != pts_.end()) {
        return it->second.get();
    }
    if (auto it = steiner_pts_.find(p); it != steiner_pts_.end()) {
        auto pt = it->second.get();
        pts_.add(p, std::move(it->second));
        steiner_pts_.erase(it);
        return pt;
    }
    auto pt = gen_point(c);
    p2t::Point* ppt = pt.get();
    pts_.add(p, std::move(pt));
    coords_.add(ppt, c);
    return ppt;
}

std::vector<p2t::Point*> P2tPointSet::remaining_steiner_points() const {
    std::vector<p2t::Point*> result;
    result.reserve(steiner_pts_.size());
    for (auto& [_, p] : steiner_pts_) {
        result.push_back(p.get());
    }
    return result;
}

FixedArray<CompressedScenePos, 2> P2tPointSet::compute_coords(const p2t::Point* p) const {
    return { (CompressedScenePos)(p->x / scale_), (CompressedScenePos)(p->y / scale_) };
}

const FixedArray<CompressedScenePos, 2>* P2tPointSet::try_get_coords(const p2t::Point* p) const {
    return coords_.try_get(p);
}

void P2tPointSet::print(std::ostream& ostr) const {
    ostr << "P2tPointSet\n";
    ostr << "  Points\n";
    for (const auto& [c, p] : pts_) {
        ostr << "    " << c << " " << p.get() << '\n';
    }
    ostr << "  Steiner\n";
    for (const auto& [c, p] : steiner_pts_) {
        ostr << "    " << c << " " << p.get() << '\n';
    }
    ostr << "  Coordinates\n";
    for (const auto& [p, c] : coords_) {
        ostr << "    " << c << " " << p << '\n';
    }
}

std::ostream& operator << (std::ostream& ostr, const P2tPointSet& p) {
    p.print(ostr);
    return ostr;
}
