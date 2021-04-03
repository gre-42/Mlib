#pragma once
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct Node;
struct Way;
enum class TerrainType;
template <typename TData, size_t... tshape>
class FixedArray;

std::list<std::pair<TerrainType, std::list<FixedArray<float, 3>>>> get_region_contours(
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways);

}
