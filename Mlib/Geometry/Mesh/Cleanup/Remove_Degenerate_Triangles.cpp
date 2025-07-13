#include "Remove_Degenerate_Triangles.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Largest_Cosine.hpp>
#include <vector>

using namespace Mlib;

template <class TPos>
void Mlib::remove_degenerate_triangles(ColoredVertexArray<TPos>& cva) {
    std::vector<FixedArray<ColoredVertex<TPos>, 3>> triangles;
    std::vector<FixedArray<float, 3>> alpha;
    triangles.reserve(cva.triangles.size());
    alpha.reserve(cva.alpha.size());
    if (!cva.alpha.empty() && (cva.alpha.size() != cva.triangles.size())) {
        THROW_OR_ABORT("Conflicting number of alphas");
    }
    for (size_t i = 0; i < cva.triangles.size(); ++i) {
        const auto& tri = cva.triangles[i];
        using Triangle = FixedArray<TPos, 3, 3>;
        auto tlc = triangle_largest_cosine(funpack(Triangle{
            tri(0).position,
            tri(1).position,
            tri(2).position}));
        if (std::isnan(tlc) || (tlc > (1 - 1e-7f))) {
            continue;
        }
        triangles.push_back(tri);
        if (!cva.alpha.empty()) {
            alpha.push_back(cva.alpha[i]);
        }
    }
    cva.triangles = uuvector(triangles.begin(), triangles.end());
    cva.alpha = uuvector(alpha.begin(), alpha.end());
}

template void Mlib::remove_degenerate_triangles<float>(ColoredVertexArray<float>& cva);
template void Mlib::remove_degenerate_triangles<CompressedScenePos>(ColoredVertexArray<CompressedScenePos>& cva);
