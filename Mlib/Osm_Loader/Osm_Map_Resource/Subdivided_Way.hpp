#pragma once
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct Node;
template <typename TData, size_t... tshape>
class FixedArray;

std::list<FixedArray<double, 2>> subdivided_way(
    const std::map<std::string, Node>& nodes,
    const std::list<std::string>& nd,
    double scale,
    double max_length);

}
