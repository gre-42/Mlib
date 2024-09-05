#include "Shade_Flat.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>

using namespace Mlib;

template <class TPos>
void Mlib::shade_flat(ColoredVertexArray<TPos>& cva)
{
    for (auto& t : cva.triangles) {
        auto normal = triangle_normal<TPos>({t(0).position, t(1).position, t(2).position}).casted<float>();
        t(0).normal = normal;
        t(1).normal = normal;
        t(2).normal = normal;
    }
}

namespace Mlib {
    template void shade_flat<float>(
        ColoredVertexArray<float>& cvas);
    template void shade_flat<double>(
        ColoredVertexArray<double>& cvas);
}
