#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <map>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct Node;

void project_nodes_onto_ways(
    std::map<std::string, Node>& nodes,
    const std::list<FixedArray<CompressedScenePos, 2, 2>>& way_segments,
    double scale);

}
