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
{}

#pragma GCC push_options
#pragma GCC optimize ("O3")

std::vector<FixedArray<float, 3>> ColoredVertexArray::vertices() const {
    std::vector<FixedArray<float, 3>> res;
    res.reserve(triangles.size() * 3);
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
            ColoredVertex{
                position: dehomogenized_3(dot1d(m, homogenized_4(tri(0).position))),
                color: tri(0).color,
                uv: tri(0).uv,
                normal: dot1d(R3_from_4x4(m), tri(0).normal)
            },
            ColoredVertex{
                position: dehomogenized_3(dot1d(m, homogenized_4(tri(1).position))),
                color: tri(1).color,
                uv: tri(1).uv,
                normal: dot1d(R3_from_4x4(m), tri(1).normal)
            },
            ColoredVertex{
                position: dehomogenized_3(dot1d(m, homogenized_4(tri(2).position))),
                color: tri(2).color,
                uv: tri(2).uv,
                normal: dot1d(R3_from_4x4(m), tri(2).normal)
            }});
    }
    res->lines.reserve(lines.size());
    for(const auto& li : lines) {
        res->lines.push_back({
            ColoredVertex{
                position: dehomogenized_3(dot1d(m, homogenized_4(li(0).position))),
                color: li(0).color,
                uv: li(0).uv,
                normal: dot1d(R3_from_4x4(m), li(0).normal)
            },
            ColoredVertex{
                position: dehomogenized_3(dot1d(m, homogenized_4(li(1).position))),
                color: li(1).color,
                uv: li(1).uv,
                normal: dot1d(R3_from_4x4(m), li(1).normal)
            }});
    }
    return res;
}

std::vector<CollisionTriangle> ColoredVertexArray::transformed_triangles(const FixedArray<float, 4, 4>& m) const {
    std::vector<CollisionTriangle> res;
    res.reserve(triangles.size());
    for(const auto& t : triangles) {
        FixedArray<FixedArray<float, 3>, 3> pt{
            dehomogenized_3(dot1d(m, homogenized_4(t(0).position))),
            dehomogenized_3(dot1d(m, homogenized_4(t(1).position))),
            dehomogenized_3(dot1d(m, homogenized_4(t(2).position)))};
        res.push_back(CollisionTriangle{
            bounding_sphere: BoundingSphere{pt},
            plane: PlaneNd<float, 3>{pt},
            two_sided: !material.cull_faces,
            triangle: pt});
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

#pragma GCC pop_options
