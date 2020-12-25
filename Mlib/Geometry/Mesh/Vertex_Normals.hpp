#pragma once
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <map>

namespace Mlib {

class VertexNormals {
public:
    template <class TIterator>
    inline void add_triangles(const TIterator& begin, const TIterator& end) {
        for (auto it = begin; it != end; ++it) {
            add_triangle(*it);
        }
    }
    inline void add_triangle(const FixedArray<ColoredVertex, 3>& triangle) {
        for (auto& v : triangle.flat_iterable()) {
            add_vertex_face_normal(v.position, v.normal);
        }
    }
    inline void add_vertex_face_normal(
        const FixedArray<float, 3>& position,
        const FixedArray<float, 3>& normal)
    {
        vertices_[OrderableFixedArray{position}] += normal;
    }
    inline void compute_vertex_normals() {
        for (auto& v : vertices_) {
            v.second /= std::sqrt(sum(squared(v.second)));
        }
    }
    inline const FixedArray<float, 3>& get_normal(const FixedArray<float, 3>& position) {
        return vertices_.at(OrderableFixedArray{position});
    }
    inline const std::map<OrderableFixedArray<float, 3>, FixedArray<float, 3>>& vertices() {
        return vertices_;
    }
private:
    std::map<OrderableFixedArray<float, 3>, FixedArray<float, 3>> vertices_;
};

}
