#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Vertex_Normals.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <list>
#include <string>

namespace Mlib {

template <class TIndex>
struct IndexVertex {
    TIndex position;
    TIndex uv;
    TIndex normal;
};

template <class TInputTriangles, class TInputQuads>
struct NamedInputPolygons {
    std::string name;
    std::string material_name;
    const TInputTriangles& triangles;
    const TInputQuads& quads;
};

template <class TIndex>
struct NamedObjPolygons {
    std::string name;
    std::string material_name;
    UUVector<FixedArray<IndexVertex<TIndex>, 3>> triangles;
    UUVector<FixedArray<IndexVertex<TIndex>, 4>> quads;
};

template <class TDir, class TPos, class TIndex>
class IndexedFaceSet {
    using NamedListInputPolygons = NamedInputPolygons<
        std::list<FixedArray<ColoredVertex<TPos>, 3>>,
        std::list<FixedArray<ColoredVertex<TPos>, 4>>>;
    using NamedVectorInputPolygons = NamedInputPolygons<
        UUVector<FixedArray<ColoredVertex<TPos>, 3>>,
        UUVector<FixedArray<ColoredVertex<TPos>, 4>>>;
public:
    IndexedFaceSet(
        const std::list<FixedArray<ColoredVertex<TPos>, 3>>& triangles,
        const std::list<FixedArray<ColoredVertex<TPos>, 4>>& quads = {})
    : IndexedFaceSet{std::vector<NamedListInputPolygons>{NamedListInputPolygons{"", "", triangles, quads}}}
    {}
    IndexedFaceSet(
        const UUVector<FixedArray<ColoredVertex<TPos>, 3>>& triangles,
        const UUVector<FixedArray<ColoredVertex<TPos>, 4>>& quads = {})
    : IndexedFaceSet{std::vector<NamedVectorInputPolygons>{NamedVectorInputPolygons{"", "", triangles, quads}}}
    {}
    template <class TInputTriangles, class TInputQuads>
    IndexedFaceSet(const std::vector<NamedInputPolygons<TInputTriangles, TInputQuads>>& named_input_polygons)
    {
        std::map<OrderableFixedArray<TPos, 3>, size_t> vertex_indices;
        std::map<OrderableFixedArray<TDir, 3>, size_t> normal_indices;
        std::map<OrderableFixedArray<TDir, 2>, size_t> uv_indices;
        named_obj_polygons.reserve(named_input_polygons.size());
        for (const NamedInputPolygons<TInputTriangles, TInputQuads>& named_ipolys : named_input_polygons) {
            auto& named_opolys = named_obj_polygons.emplace_back(NamedObjPolygons<TIndex>{named_ipolys.name, named_ipolys.material_name, {}, {}});
            named_opolys.triangles.reserve(named_ipolys.triangles.size());
            named_opolys.quads.reserve(named_ipolys.quads.size());
            auto add_polygons = [&](const auto& ipolys, auto& opolys)
            {
                for (const auto& ipoly : ipolys) {
                    for (const auto& v : ipoly.flat_iterable()) {
                        vertex_indices.insert({ OrderableFixedArray(v.position), vertex_indices.size() });
                        uv_indices.insert({ OrderableFixedArray(v.uv), uv_indices.size() });
                        normal_indices.insert({ OrderableFixedArray(v.normal), normal_indices.size() });
                    }
                    FixedArray<IndexVertex<TIndex>, std::remove_reference_t<decltype(ipoly)>::length()> opolygon{ uninitialized };
                    for (size_t i = 0; i < CW::length(ipoly); ++i) {
                        opolygon(i).position = vertex_indices.at(OrderableFixedArray(ipoly(i).position));
                        opolygon(i).uv = uv_indices.at(OrderableFixedArray(ipoly(i).uv));
                        opolygon(i).normal = normal_indices.at(OrderableFixedArray(ipoly(i).normal));
                    }
                    opolys.emplace_back(opolygon);
                }
            };
            add_polygons(named_ipolys.triangles, named_opolys.triangles);
            add_polygons(named_ipolys.quads, named_opolys.quads);
        }
        vertices.resize(vertex_indices.size());
        for (const auto& v : vertex_indices) {
            vertices[v.second] = v.first;
        }
        uvs.resize(uv_indices.size());
        for (const auto& v : uv_indices) {
            uvs[v.second] = v.first;
        }
        normals.resize(normal_indices.size());
        for (const auto& v : normal_indices) {
            normals[v.second] = v.first;
        }
    }
    UUVector<FixedArray<TPos, 3>> vertices;
    UUVector<FixedArray<TDir, 2>> uvs;
    UUVector<FixedArray<TDir, 3>> normals;
    UVector<NamedObjPolygons<TIndex>> named_obj_polygons;
};

}
