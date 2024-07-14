#pragma once
#include <list>
#include <memory>
#include <string>

namespace Mlib {

struct Material;
struct Morphology;
struct BarrierStyle;
template <class TPos>
struct ColoredVertex;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TPos>
class TriangleList;

void draw_boundary_barriers(
    std::list<std::shared_ptr<TriangleList<double>>>& tls,
    const std::list<FixedArray<ColoredVertex<double>, 3>>& inner_triangles,
    const Material& material,
    const Morphology& morphology,
    float scale,
    float uv_scale,
    float barrier_height,
    const BarrierStyle& barrier_style);

}
