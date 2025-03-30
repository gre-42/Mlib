#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <map>
#include <string>

namespace p2t {
    struct Point;
}

namespace Mlib {

struct Node;
template <typename TData, size_t... tshape>
class FixedArray;

double compute_area_clockwise(
    const std::list<std::string>& nd,
    const std::map<std::string, Node>& nodes,
    double scale);

double compute_area_ccw(
    const std::vector<p2t::Point*>& polygon,
    double scale);

double compute_area_ccw(
    const std::list<FixedArray<CompressedScenePos, 2>>& polygon,
    double scale);
    
}
