#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>

namespace Mlib {

struct Node;

class WayBvh {
    typedef FixedArray<FixedArray<double, 2>, 2> Line2d;
public:
    WayBvh();
    explicit WayBvh(const std::list<Line2d>& way_segments);
    ~WayBvh();
    void add_path(const std::list<FixedArray<double, 2>>& path);
    void nearest_way(
        const FixedArray<double, 2>& position,
        double max_dist,
        FixedArray<double, 2>& dir,
        double& distance) const;
    FixedArray<double, 2> project_onto_way(
        const std::string& node_id,
        const Node& node,
        double scale) const;
private:
    Bvh<double, Line2d, 2> bvh_;
};

}
