#pragma once
#include <list>
#include <vector>

namespace Mlib {

class TriangleList;
struct BoundingInfo;
struct ColoredVertex;
struct SteinerPointInfo;
template <typename TData, size_t... tshape>
class FixedArray;

void triangulate_terrain_or_ceilings(
    TriangleList& tl_terrain,
    TriangleList* tl_terrain_visuals,
    const std::list<std::list<FixedArray<ColoredVertex, 3>>>& tl_insert,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const std::vector<FixedArray<float, 2>>& bounding_contour,
    const std::list<FixedArray<ColoredVertex, 3>>& hole_triangles,
    float scale,
    float uv_scale,
    float z,
    const FixedArray<float, 3>& color);

}
