#pragma once
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct Rectangle;
struct Building;
struct Node;
template <typename TData, size_t... tshape>
class FixedArray;
enum class DrawBuildingPartType;

std::list<FixedArray<FixedArray<float, 2>, 2>> smooth_building_level(
    const Building& bu,
    const std::map<std::string, Node>& nodes,
    float max_length,
    float width0,
    float width1,
    float scale);

std::list<FixedArray<float, 2>> smooth_building_level_outline(
    const Building& bu,
    const std::map<std::string, Node>& nodes,
    float scale,
    float max_length,
    DrawBuildingPartType tpe);

}
