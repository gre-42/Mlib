#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct Node;
struct Way;
enum class WaterType;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TRegionType, class TGeometry>
struct RegionWithMargin;

std::list<RegionWithMargin<WaterType, std::list<FixedArray<CompressedScenePos, 2>>>> get_water_region_contours(
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways);

}
