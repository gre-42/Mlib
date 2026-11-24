#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <list>
#include <vector>

namespace p2t {
    struct Point;
}

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

double compute_area_ccw(
    const std::vector<p2t::Point*>& polygon,
    double scale);

double compute_area_ccw(
    const std::vector<FixedArray<CompressedScenePos, 2>>& polygon,
    double scale);

double compute_area_ccw(
    const std::list<FixedArray<CompressedScenePos, 2>>& polygon,
    double scale);

}
