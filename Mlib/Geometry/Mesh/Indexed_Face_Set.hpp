#pragma once
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Vertex_Normals.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <list>
#include <string>

namespace Mlib {

template <class TIndex>
struct IndexVertex {
    TIndex position;
    TIndex normal;
};

template <class TInputTriangles>
struct NamedInputTriangles {
    std::string name;
    const TInputTriangles& triangles;
};

template <class TIndex>
struct NamedObjTriangles {
    std::string name;
    std::vector<FixedArray<IndexVertex<TIndex>, 3>> triangles;
};

template <class TData, class TIndex>
class IndexedFaceSet {
    using NamedListInputTriangles = NamedInputTriangles<std::list<FixedArray<ColoredVertex, 3>>>;
    using NamedVectorInputTriangles = NamedInputTriangles<std::vector<FixedArray<ColoredVertex, 3>>>;
public:
    IndexedFaceSet(const std::list<FixedArray<ColoredVertex, 3>>& triangles)
    : IndexedFaceSet{std::vector<NamedListInputTriangles>{NamedListInputTriangles{"", triangles}}}
    {}
    IndexedFaceSet(const std::vector<FixedArray<ColoredVertex, 3>>& triangles)
    : IndexedFaceSet{std::vector<NamedVectorInputTriangles>{NamedVectorInputTriangles{"", triangles}}}
    {}
    template <class TInputTriangles>
    IndexedFaceSet(const std::vector<NamedInputTriangles<TInputTriangles>>& named_input_triangles)
    {
        std::map<OrderableFixedArray<float, 3>, size_t> vertex_indices;
        std::map<OrderableFixedArray<float, 3>, size_t> normal_indices;
        named_obj_triangles.reserve(named_input_triangles.size());
        for (const NamedInputTriangles<TInputTriangles>& itris : named_input_triangles) {
            named_obj_triangles.push_back(NamedObjTriangles<TIndex>{itris.name, {}});
            std::vector<FixedArray<IndexVertex<TIndex>, 3>>& otris = named_obj_triangles.back().triangles;
            otris.reserve(itris.triangles.size());
            for (auto& tri : itris.triangles) {
                for (auto& v : tri.flat_iterable()) {
                    vertex_indices.insert({OrderableFixedArray{v.position}, vertex_indices.size()});
                    normal_indices.insert({OrderableFixedArray{v.normal}, normal_indices.size()});
                }
                FixedArray<IndexVertex<TIndex>, 3> triangle;
                for (size_t i = 0; i < 3; ++i) {
                    triangle(i).position = vertex_indices.at(OrderableFixedArray{tri(i).position});
                    triangle(i).normal = normal_indices.at(OrderableFixedArray{tri(i).normal});
                }
                otris.push_back(triangle);
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
    }
    std::vector<FixedArray<TData, 3>> vertices;
    std::vector<FixedArray<TData, 3>> normals;
    std::vector<NamedObjTriangles<TIndex>> named_obj_triangles;
};

}
