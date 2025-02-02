#pragma once
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <list>

namespace Mlib {

struct Node;

class WayBvh {
    typedef FixedArray<CompressedScenePos, 2, 2> Line2d;
public:
    WayBvh();
    explicit WayBvh(const std::list<Line2d>& way_segments);
    ~WayBvh();
    void add_path(const std::list<FixedArray<CompressedScenePos, 2>>& path);
    [[nodiscard]] bool nearest_way(
        const FixedArray<CompressedScenePos, 2>& position,
        CompressedScenePos max_dist,
        FixedArray<double, 2>& dir,
        CompressedScenePos& distance) const;
    FixedArray<CompressedScenePos, 2> project_onto_way(
        const std::string& node_id,
        const Node& node,
        double scale) const;
private:
    Bvh<CompressedScenePos, 2, Line2d> bvh_;
};

}
