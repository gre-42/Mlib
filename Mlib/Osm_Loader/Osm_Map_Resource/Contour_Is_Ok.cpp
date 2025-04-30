#include "Contour_Is_Ok.hpp"
#include <Mlib/Geometry/Intersection/Contour_Intersections.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Close_Neighbor_Detector.hpp>

using namespace Mlib;

static bool contains_close_neighbors(
    const std::vector<FixedArray<CompressedScenePos, 2>>& points,
    CompressedScenePos min_distance)
{
    CloseNeighborDetector<CompressedScenePos, 2> close_neighbor_detector{{(CompressedScenePos)10., (CompressedScenePos)10.}, 10};
    for (const auto& p : points) {
        if (close_neighbor_detector.contains_neighbor(
            p,
            min_distance))
        {
            return true;
        }
    }
    return false;
}

bool Mlib::contour_is_ok(
    const std::vector<FixedArray<CompressedScenePos, 2>>& contour,
    CompressedScenePos min_distance)
{
    return
        !contains_close_neighbors(contour, min_distance) &&
        visit_contour_intersections(
            {contour},
            [md = (ScenePos)min_distance]
            (const auto& position, size_t i, size_t j, ScenePos distance)
            { return distance >= md; });
}
