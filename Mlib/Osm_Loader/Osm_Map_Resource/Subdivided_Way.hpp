#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct Node;
template <typename TData, size_t... tshape>
class FixedArray;
struct SubdividedWayVertex;

std::list<SubdividedWayVertex> subdivided_way(
    const std::map<std::string, Node>& nodes,
    const std::list<std::string>& nd,
    double scale,
    double max_length);

}
