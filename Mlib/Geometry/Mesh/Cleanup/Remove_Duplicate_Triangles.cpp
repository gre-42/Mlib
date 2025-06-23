#include "Remove_Duplicate_Triangles.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <list>
#include <set>
#include <tuple>

using namespace Mlib;

template <class TPos>
void Mlib::remove_duplicate_triangles(ColoredVertexArray<TPos>& cva) {
    std::list<FixedArray<ColoredVertex<TPos>, 3>> trimmed;
    std::set<std::tuple<OrderableFixedArray<TPos, 3>, OrderableFixedArray<TPos, 3>, OrderableFixedArray<TPos, 3>>> first_triangles;
    for (const auto& tri : cva.triangles) {
        if (!first_triangles.emplace(
            tri(0).position,
            tri(1).position,
            tri(2).position).second)
        {
            linfo() << "Found duplicate triangle";
            continue;
        }
        trimmed.push_back(tri);
    }
    cva.triangles = uuvector(trimmed.begin(), trimmed.end());
}

namespace Mlib {
    template void remove_duplicate_triangles<float>(ColoredVertexArray<float>& cva);
    template void remove_duplicate_triangles<double>(ColoredVertexArray<double>& cva);
}
