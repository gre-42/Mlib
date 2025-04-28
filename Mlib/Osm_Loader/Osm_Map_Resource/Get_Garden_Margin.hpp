#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <map>

namespace Mlib {

struct Building;
struct Node;

template <typename TData, size_t... tshape>
class OrderableFixedArray;

std::map<OrderableFixedArray<CompressedScenePos, 2>, CompressedScenePos> get_garden_margin(
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    double scale,
    double max_length);

}
