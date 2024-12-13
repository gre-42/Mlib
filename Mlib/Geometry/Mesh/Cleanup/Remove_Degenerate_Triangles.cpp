#include "Remove_Degenerate_Triangles.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Largest_Cosine.hpp>
#include <vector>

using namespace Mlib;

template <class TPos>
void Mlib::remove_degenerate_triangles(ColoredVertexArray<TPos>& cva) {
    std::vector<FixedArray<ColoredVertex<TPos>, 3>> trimmed;
    trimmed.reserve(cva.triangles.size());
    for (const auto& tri : cva.triangles) {
        using Triangle = FixedArray<TPos, 3, 3>;
        auto tlc = triangle_largest_cosine(funpack(Triangle{
            tri(0).position,
            tri(1).position,
            tri(2).position}));
        if (std::isnan(tlc) || (tlc > (1 - 1e-7f))) {
            continue;
        }
        trimmed.push_back(tri);
    }
    cva.triangles = uuvector(trimmed.begin(), trimmed.end());
}

template void Mlib::remove_degenerate_triangles<float>(ColoredVertexArray<float>& cva);
template void Mlib::remove_degenerate_triangles<CompressedScenePos>(ColoredVertexArray<CompressedScenePos>& cva);
