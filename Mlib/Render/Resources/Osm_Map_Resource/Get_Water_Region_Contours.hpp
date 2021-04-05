#pragma once
#include <list>
#include <map>
#include <string>

namespace Mlib {

struct Node;
struct Way;
enum class WaterType;
template <typename TData, size_t... tshape>
class FixedArray;

std::list<std::pair<WaterType, std::list<FixedArray<float, 3>>>> get_water_region_contours(
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways);

}
