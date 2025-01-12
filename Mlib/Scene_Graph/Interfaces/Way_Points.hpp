#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Scene_Graph/Interfaces/Way_Points_Fwd.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>

namespace Mlib {

struct WayPointsAndBvh {
    explicit WayPointsAndBvh(PointsAndAdjacencyResource way_points);
    PointsAndAdjacencyResource way_points;
    Bvh<CompressedScenePos, 3, size_t> bvh;
};

}
