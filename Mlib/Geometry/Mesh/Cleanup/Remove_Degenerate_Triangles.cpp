#include "Remove_Degenerate_Triangles.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Largest_Cosine.hpp>
#include <list>

using namespace Mlib;

template <class TPos>
void Mlib::remove_degenerate_triangles(ColoredVertexArray<TPos>& cva) {
    std::list<FixedArray<ColoredVertex<TPos>, 3>> trimmed;
    for (const auto& tri : cva.triangles) {
        auto tlc = triangle_largest_cosine<TPos, 3>({
            tri(0).position,
            tri(1).position,
            tri(2).position});
        if (std::isnan(tlc) || (tlc > (TPos)(1 - 1e-7))) {
            continue;
        }
        trimmed.push_back(tri);
    }
    cva.triangles = std::vector(trimmed.begin(), trimmed.end());
}

namespace Mlib {
    template void remove_degenerate_triangles<float>(ColoredVertexArray<float>& cva);
    template void remove_degenerate_triangles<double>(ColoredVertexArray<double>& cva);
}
