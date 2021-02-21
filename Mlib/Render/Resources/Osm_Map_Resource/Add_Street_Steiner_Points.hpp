#pragma once
#include <list>
#include <vector>

namespace Mlib {

struct SteinerPointInfo;
template <typename TData, size_t... tshape>
class FixedArray;
struct ColoredVertex;
struct BoundingInfo;

void add_street_steiner_points(
    std::list<SteinerPointInfo>& steiner_points,
    const std::list<FixedArray<ColoredVertex, 3>>& ground_triangles,
    const std::list<FixedArray<ColoredVertex, 3>>& air_triangles,
    const BoundingInfo& bounding_info,
    float scale,
    const std::vector<float>& steiner_point_distances_road,
    const std::vector<float>& steiner_point_distances_steiner);

}
