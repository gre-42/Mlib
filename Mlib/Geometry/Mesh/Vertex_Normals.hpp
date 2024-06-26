#pragma once
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <map>

namespace Mlib {

template <class TPos, class TNormal>
class VertexNormals {
public:
    template <class TIterator>
    inline void add_triangles(const TIterator& begin, const TIterator& end) {
        for (auto it = begin; it != end; ++it) {
            add_triangle(*it);
        }
    }
    inline void add_triangle(const FixedArray<ColoredVertex<TPos>, 3>& triangle) {
        for (auto& v : triangle.flat_iterable()) {
            add_vertex_face_normal(v.position, v.normal);
        }
    }
    inline void add_vertex_face_normal(
        const FixedArray<TPos, 3>& position,
        const FixedArray<TNormal, 3>& normal)
    {
        auto it = vertices_.try_emplace(OrderableFixedArray{position}, TNormal(0)).first;
        it->second += normal;
    }
    inline void compute_vertex_normals() {
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
private:
    std::map<OrderableFixedArray<TPos, 3>, FixedArray<TNormal, 3>> vertices_;
};

}
