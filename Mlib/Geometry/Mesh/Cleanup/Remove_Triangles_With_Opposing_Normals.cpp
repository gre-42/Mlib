#include "Remove_Triangles_With_Opposing_Normals.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <list>
#include <map>

using namespace Mlib;

template <class TPos>
void Mlib::remove_triangles_with_opposing_normals(ColoredVertexArray<TPos>& cva) {
    std::map<std::pair<OrderableFixedArray<TPos, 3>, OrderableFixedArray<TPos, 3>>, size_t> edges;
    for (const auto& tri : cva.triangles) {
        for (size_t i = 0; i < 3; ++i) {
            ++edges[{make_orderable(tri(i).position), make_orderable(tri((i + 1) % 3).position)}];
        }
    }
    std::list<FixedArray<ColoredVertex<TPos>, 3>> trimmed;
    for (const auto& tri : cva.triangles) {
        for (size_t i = 0; i < 3; ++i) {
            if (edges.at({make_orderable(tri(i).position), make_orderable(tri((i + 1) % 3).position)}) >= 2) {
                goto skip;
            }
        }
        trimmed.push_back(tri);
        skip:;
    }
    cva.triangles = uuvector(trimmed.begin(), trimmed.end());
}

namespace Mlib {
    template void remove_triangles_with_opposing_normals<float>(ColoredVertexArray<float>& cva);
    template void remove_triangles_with_opposing_normals<double>(ColoredVertexArray<double>& cva);
}
