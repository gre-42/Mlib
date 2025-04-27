#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct Node;
struct Way;
enum class TerrainType;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TRegionType, class TGeometry>
struct RegionWithMargin;

std::list<RegionWithMargin<TerrainType, std::list<FixedArray<CompressedScenePos, 2>>>> get_terrain_region_contours(
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways);

}
