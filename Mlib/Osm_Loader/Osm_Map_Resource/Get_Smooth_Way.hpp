#pragma once
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct Node;
template <typename TData, size_t... tshape>
class FixedArray;

std::list<FixedArray<float, 2>> smooth_way(
    const std::map<std::string, Node>& nodes,
    const std::list<std::string>& nd,
    float scale,
    float max_length);

}
