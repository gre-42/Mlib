#pragma once
#include <Mlib/Map/Map.hpp>
#include <Mlib/Map/Verbose_Unordered_Map.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <map>
#include <memory>
#include <vector>

namespace p2t {

struct Point;

}

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TData, size_t... tshape>
class OrderableFixedArray;

class P2tPointSet {
public:
    P2tPointSet(const std::list<FixedArray<CompressedScenePos, 2>>& steiner_points, double scale);
    ~P2tPointSet();
    p2t::Point* operator () (CompressedScenePos x, CompressedScenePos y);
    std::vector<p2t::Point*> remaining_steiner_points() const;
    const FixedArray<CompressedScenePos, 2>& coords(const p2t::Point* p) const;
private:
    std::unique_ptr<p2t::Point> gen_point(CompressedScenePos x, CompressedScenePos y) const;
    double scale_;
    Map<OrderableFixedArray<CompressedScenePos, 2>, std::unique_ptr<p2t::Point>> pts_;
    Map<OrderableFixedArray<CompressedScenePos, 2>, std::unique_ptr<p2t::Point>> steiner_pts_;
    VerboseUnorderedMap<const p2t::Point*, FixedArray<CompressedScenePos, 2>> coords_;
};

}
