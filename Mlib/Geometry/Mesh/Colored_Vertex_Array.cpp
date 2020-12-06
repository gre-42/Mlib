#include "Colored_Vertex_Array.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

ColoredVertexArray::ColoredVertexArray(
    const std::string& name,
    const Material& material,
    std::vector<FixedArray<ColoredVertex, 3>>&& triangles,
    std::vector<FixedArray<ColoredVertex, 2>>&& lines)
: name{name},
  material{material},
  triangles{std::forward<std::vector<FixedArray<ColoredVertex, 3>>>(triangles)},
  lines{std::forward<std::vector<FixedArray<ColoredVertex, 2>>>(lines)}
{
    assert_true(!name.empty());
}

#pragma GCC push_options
#pragma GCC optimize ("O3")

std::vector<FixedArray<float, 3>> ColoredVertexArray::vertices() const {
    std::vector<FixedArray<float, 3>> res;
    res.reserve(triangles.size() * 3 + lines.size() * 2);
    for(auto& v : triangles) {
        res.push_back(v(0).position);
        res.push_back(v(1).position);
        res.push_back(v(2).position);
    }
    for(auto& v : lines) {
        res.push_back(v(0).position);
        res.push_back(v(1).position);
    }
    return res;
}

std::shared_ptr<ColoredVertexArray> ColoredVertexArray::transformed(const FixedArray<float, 4, 4>& m) const {
    auto res = std::make_shared<ColoredVertexArray>();
    res->material = material;
    res->triangles.reserve(triangles.size());
    for(const auto& tri : triangles) {
        res->triangles.push_back({
            tri(0).transformed(m),
            tri(1).transformed(m),
            tri(2).transformed(m)});
    }
    res->lines.reserve(lines.size());
    for(const auto& li : lines) {
        res->lines.push_back({
            li(0).transformed(m),
            li(1).transformed(m)});
    }
    return res;
}

std::vector<CollisionTriangleSphere> ColoredVertexArray::transformed_triangles_sphere(const FixedArray<float, 4, 4>& m) const {
    std::vector<CollisionTriangleSphere> res;
    res.reserve(triangles.size());
    for(const auto& t : triangles) {
        FixedArray<FixedArray<float, 3>, 3> pt{
            dehomogenized_3(dot1d(m, homogenized_4(t(0).position))),
            dehomogenized_3(dot1d(m, homogenized_4(t(1).position))),
            dehomogenized_3(dot1d(m, homogenized_4(t(2).position)))};
        res.push_back(CollisionTriangleSphere{
            .bounding_sphere = BoundingSphere<float, 3>{pt},
            .plane = PlaneNd<float, 3>{pt},
            .two_sided = !material.cull_faces,
            .triangle = pt});
    }
    return res;
}

std::vector<CollisionTriangleBbox> ColoredVertexArray::transformed_triangles_bbox(const FixedArray<float, 4, 4>& m) const {
    std::vector<CollisionTriangleBbox> res;
    res.reserve(triangles.size());
    for(const auto& t : triangles) {
        FixedArray<FixedArray<float, 3>, 3> pt{
            dehomogenized_3(dot1d(m, homogenized_4(t(0).position))),
            dehomogenized_3(dot1d(m, homogenized_4(t(1).position))),
            dehomogenized_3(dot1d(m, homogenized_4(t(2).position)))};
        res.push_back(CollisionTriangleBbox{
            .base = CollisionTriangleSphere{
                .bounding_sphere = BoundingSphere<float, 3>{pt},
                .plane = PlaneNd<float, 3>{pt},
                .two_sided = !material.cull_faces,
                .triangle = pt
            },
            .bounding_box = BoundingBox<float, 3>{pt}});
    }
    return res;
}

std::vector<FixedArray<FixedArray<float, 3>, 2>> ColoredVertexArray::transformed_lines(const FixedArray<float, 4, 4>& m) const {
    std::vector<FixedArray<FixedArray<float, 3>, 2>> res;
    res.reserve(lines.size());
    for(const auto& t : lines) {
        res.push_back(FixedArray<FixedArray<float, 3>, 2>{
            dehomogenized_3(dot1d(m, homogenized_4(t(0).position))),
            dehomogenized_3(dot1d(m, homogenized_4(t(1).position)))});
    }
    return res;
}

void Mlib::sort_for_rendering(std::list<std::shared_ptr<ColoredVertexArray>>& colored_vertex_arrays) {
    colored_vertex_arrays.sort([](
        const std::shared_ptr<ColoredVertexArray>& a,
        const std::shared_ptr<ColoredVertexArray>& b)
        {
            return a->material.blend_mode < b->material.blend_mode;
        });
}

#pragma GCC pop_options
