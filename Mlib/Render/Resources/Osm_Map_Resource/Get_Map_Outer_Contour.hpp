#pragma once
#include <map>
#include <string>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct Node;
struct Way;

std::vector<FixedArray<float, 2>> get_map_outer_contour(
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways);

}
