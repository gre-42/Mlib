#include "Smoothen_Edges.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>
#include <map>

using namespace Mlib;

template <class TPos>
void Mlib::smoothen_edges(
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const std::set<OrderableFixedArray<TPos, 3>>& excluded_vertices,
    float smoothness,
    size_t niterations,
    float decay)
{
    for (size_t i = 0; i < niterations; ++i) {
        typedef OrderableFixedArray<TPos, 3> Vertex3;
        typedef OrderableFixedArray<Vertex3, 2> Edge3;
        std::map<Edge3, Vertex3> edge_neighbors;
        std::map<Vertex3, FixedArray<TPos, 3>> vertex_movement;
        for (const auto& l : cvas) {
            for (const auto& t : l->triangles) {
                auto insert_edge = [&](size_t i, size_t j, size_t n){
                    Vertex3 ei{t(i).position};
                    Vertex3 ej{t(j).position};
                    Vertex3 nn{t(n).position};
                    auto it = edge_neighbors.find(Edge3{ej, ei});
                    if (it == edge_neighbors.end()) {
                        edge_neighbors.insert({Edge3{ei, ej}, nn});
                    } else {
                        FixedArray<TPos, 3> n0 = triangle_normal<TPos>({ej, ei, it->second});
                        FixedArray<TPos, 3> n1 = triangle_normal<TPos>({ei, ej, nn});
                        FixedArray<TPos, 3> cn = (it->second + nn) / TPos(2);
                        FixedArray<TPos, 3> ce = (ei + ej) / TPos(2);
                        FixedArray<TPos, 3> v = cn - ce;
                        FixedArray<TPos, 3> n01 = (n0 + n1) / TPos(2);
                        n01 /= std::sqrt(sum(squared(n01)));
                        TPos n0n1 = dot0d(n0, n1);
                        if (n0n1 >=0 && n0n1 < 1) {
                            TPos shift = std::sqrt(1 - squared(n0n1)) * sign(dot0d(v, n01));
                            if (auto e = ei; !excluded_vertices.contains(e)) {
                                if (!vertex_movement.contains(e)) {
                                    vertex_movement[e] = 0.f;
                                }
                                vertex_movement[e] += TPos(smoothness) * n01 * shift;
                            }
                            if (auto e = ej; !excluded_vertices.contains(e)) {
                                if (!vertex_movement.contains(e)) {
                                    vertex_movement[e] = 0.f;
                                }
                                vertex_movement[e] += TPos(smoothness) * n01 * shift;
                            }
                        }
                    }
                    };
                insert_edge(0, 1, 2);
                insert_edge(1, 2, 0);
                insert_edge(2, 0, 1);
            }
        }
        for (const auto& l : cvas) {
            for (auto& t : l->triangles) {
                for (auto& v : t.flat_iterable()) {
                    auto mit = vertex_movement.find(Vertex3(v.position));
                    if (mit != vertex_movement.end()) {
                        v.position += mit->second;
                    }
                }
            }
        }
        smoothness *= decay;
    }
}

namespace Mlib {
    template void smoothen_edges<float>(
        std::list<std::shared_ptr<ColoredVertexArray<float>>>& cvas,
        const std::set<OrderableFixedArray<float, 3>>& excluded_vertices,
        float smoothness,
        size_t niterations,
        float decay);
    template void smoothen_edges<double>(
        std::list<std::shared_ptr<ColoredVertexArray<double>>>& cvas,
        const std::set<OrderableFixedArray<double, 3>>& excluded_vertices,
        float smoothness,
        size_t niterations,
        float decay);
}
