#pragma once
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Vertex_Normals.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <list>

namespace Mlib {

template <class TIndex>
struct IndexVertex {
    TIndex position;
    TIndex normal;
};

template <class TData, class TIndex>
class IndexedFaceSet {
public:
    IndexedFaceSet(const std::list<FixedArray<ColoredVertex, 3>>& triangles) {
        std::map<OrderableFixedArray<float, 3>, size_t> vertex_indices;
        std::map<OrderableFixedArray<float, 3>, size_t> normal_indices;
        this->triangles.reserve(triangles.size());
        for (auto& tri : triangles) {
            for (auto& v : tri.flat_iterable()) {
                vertex_indices.insert({OrderableFixedArray{v.position}, vertex_indices.size()});
                normal_indices.insert({OrderableFixedArray{v.normal}, normal_indices.size()});
            }
            FixedArray<IndexVertex<TIndex>, 3> triangle;
            for (size_t i = 0; i < 3; ++i) {
                triangle(i).position = vertex_indices.at(OrderableFixedArray{tri(i).position});
                triangle(i).normal = normal_indices.at(OrderableFixedArray{tri(i).normal});
            }
            this->triangles.push_back(triangle);
        }
        vertices.resize(vertex_indices.size());
        for (const auto& v : vertex_indices) {
            vertices[v.second] = v.first;
        }
        normals.resize(normal_indices.size());
        for (const auto& v : normal_indices) {
            normals[v.second] = v.first;
        }
    }
    std::vector<FixedArray<TData, 3>> vertices;
    std::vector<FixedArray<TData, 3>> normals;
    std::vector<FixedArray<IndexVertex<TIndex>, 3>> triangles;
};

}
