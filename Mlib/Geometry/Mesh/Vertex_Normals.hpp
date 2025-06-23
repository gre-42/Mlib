#pragma once
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Normal_Vector_Error_Behavior.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <map>

namespace Mlib {

enum class NormalVectorErrorBehavior;

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
        auto it = vertices_.try_emplace(OrderableFixedArray(position), TNormal(0)).first;
        it->second += normal;
    }
    void compute_vertex_normals(NormalVectorErrorBehavior zero_behavior) {
        auto zb = zero_behavior;
        for (auto& [_, n] : vertices_) {
            auto len = std::sqrt(sum(squared(n)));
            if (len < 1e-12) {
                if (any(zb & NormalVectorErrorBehavior::WARN)) {
                    lwarn() << "Normal is zero";
                    zb = NormalVectorErrorBehavior::ZERO;
                }
                if (any(zb & NormalVectorErrorBehavior::THROW)) {
                    THROW_OR_ABORT("Normal is zero");
                }
                n = 0.f;
            } else {
                n /= len;
            }
        }
    }
    inline const FixedArray<TNormal, 3>& get_normal(const FixedArray<TPos, 3>& position) {
        return vertices_.at(OrderableFixedArray(position));
    }
    inline const std::map<OrderableFixedArray<TPos, 3>, FixedArray<TNormal, 3>>& vertices() {
        return vertices_;
    }
    template <size_t tnvertices>
    FixedArray<TNormal, tnvertices, 3> get_normals(
        const FixedArray<TPos, tnvertices, 3>& position)
    {
        FixedArray<TNormal, tnvertices, 3> result = uninitialized;
        for (size_t r = 0; r < tnvertices; ++r) {
            result[r] = get_normal(position[r]);
        }
        return result;
    }
private:
    std::map<OrderableFixedArray<TPos, 3>, FixedArray<TNormal, 3>> vertices_;
};

}
