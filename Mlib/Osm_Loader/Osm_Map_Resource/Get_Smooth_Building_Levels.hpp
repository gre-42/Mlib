#pragma once
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct OsmRectangle2D;
struct Building;
struct Node;
template <typename TData, size_t... tshape>
class FixedArray;
enum class DrawBuildingPartType;

struct BuildingLevelOutline {
    std::list<FixedArray<double, 2>> outline;
    double z;
};

std::list<FixedArray<FixedArray<double, 2>, 2>> smooth_building_level(
    const Building& bu,
    const std::map<std::string, Node>& nodes,
    double max_length,
    double width0,
    double width1,
    double scale);

BuildingLevelOutline smooth_building_level_outline(
    const Building& bu,
    const std::map<std::string, Node>& nodes,
    double scale,
    double max_length,
    DrawBuildingPartType tpe);

}
