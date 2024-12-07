#pragma once
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
    explicit P2tPointSet(const std::list<FixedArray<double, 2>>& steiner_points);
    ~P2tPointSet();
    p2t::Point* operator () (double x, double y);
    std::vector<p2t::Point*> remaining_steiner_points();
private:
    std::map<OrderableFixedArray<double, 2>, std::unique_ptr<p2t::Point>> pts_;
    std::map<OrderableFixedArray<double, 2>, std::unique_ptr<p2t::Point>> steiner_pts_;
};

}
