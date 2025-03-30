#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <map>
#include <string>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct Node;
struct Way;

std::vector<FixedArray<CompressedScenePos, 2>> get_map_outer_contour(
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways);

}
