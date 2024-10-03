#pragma once
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <map>

namespace Mlib {

template <class TPos, class TNormal>
class VertexNormals {
public:
    template <class TIterator>
    void add_triangles(const TIterator& begin, const TIterator& end) {
        for (auto it = begin; it != end; ++it) {
            add_triangle(*it);
        }
    }
    void add_triangle(const FixedArray<ColoredVertex<TPos>, 3>& triangle) {
        for (auto& v : triangle.flat_iterable()) {
            add_vertex_face_normal(v.position, v.normal);
        }
    }
    void add_vertex_face_normal(
        const FixedArray<TPos, 3>& position,
        const FixedArray<TNormal, 3>& normal)
    {
        auto it = vertices_.try_emplace(OrderableFixedArray{position}, TNormal(0)).first;
        it->second += normal;
    }
    void compute_vertex_normals() {
        for (auto& [_, n] : vertices_) {
            n /= std::sqrt(sum(squared(n)));
        }
    }
    inline const FixedArray<TNormal, 3>& get_normal(const FixedArray<TPos, 3>& position) {
        return vertices_.at(OrderableFixedArray{position});
    }
    inline const std::map<OrderableFixedArray<TPos, 3>, FixedArray<TNormal, 3>>& vertices() {
        return vertices_;
    }
    template <size_t tnvertices>
    FixedArray<FixedArray<TNormal, 3>, tnvertices> get_normals(
        const FixedArray<FixedArray<TPos, 3>, tnvertices>& position)
    {
        return position-> template applied<FixedArray<TNormal, 3>>(
            [this](const auto& p) { return get_normal(p); });
    }
private:
    std::map<OrderableFixedArray<TPos, 3>, FixedArray<TNormal, 3>> vertices_;
};

}
