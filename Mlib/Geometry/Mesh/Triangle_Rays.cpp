#include "Triangle_Rays.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Vertex_Normals.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

using namespace Mlib;

static FixedArray<FixedArray<float, 3>, 2> gen_ray(
    const FixedArray<float, 3>& pos,
    const FixedArray<float, 3>& normal,
    const FixedArray<float, 3>& lengths)
{
    return {
        pos - sum(abs(lengths * normal)) / sum(abs(normal)) * normal,
        pos
    };
}

std::vector<FixedArray<FixedArray<float, 3>, 2>> Mlib::generate_triangle_face_rays(
    const std::vector<FixedArray<ColoredVertex, 3>>& triangles,
    size_t npoints,
    const FixedArray<float, 3>& lengths)
{
    std::vector<FixedArray<FixedArray<float, 3>, 2>> res;
    res.reserve((npoints * (npoints + 1)) / 2 * triangles.size());
    size_t npoints2 = npoints + 2;
    for(const auto& t : triangles) {
        for(size_t u = 1; u < npoints2; ++u) {
            for(size_t v = 1; v < npoints2 - u; ++v) {
                if ((u + v) >= npoints2) {
                    break;
                }
                float a = float(u) / npoints2;
                float b = float(v) / npoints2;
                float c = float(npoints2 - u - v) / npoints2;
                FixedArray<float, 3> pos =
                    t(0).position * a +
                    t(1).position * b +
                    t(2).position * c;
                auto normal = triangle_normal({
                    t(0).position,
                    t(1).position,
                    t(2).position});
                res.push_back(gen_ray(pos, normal, lengths));
            }
        }
    }
    return res;
}

std::vector<FixedArray<FixedArray<float, 3>, 2>> Mlib::generate_triangle_vertex_rays(
    const std::vector<FixedArray<ColoredVertex, 3>>& triangles,
    const FixedArray<float, 3>& lengths)
{
    VertexNormals vertex_normals;
    vertex_normals.add_triangles(triangles.begin(), triangles.end());
    vertex_normals.compute_vertex_normals();

    std::vector<FixedArray<FixedArray<float, 3>, 2>> res;
    res.reserve(vertex_normals.vertices().size());
    for(const auto& v : vertex_normals.vertices()) {
        res.push_back(gen_ray(v.first, v.second, lengths));
    }
    return res;
}

std::vector<FixedArray<FixedArray<float, 3>, 2>> Mlib::generate_triangle_rays(
    const std::vector<FixedArray<ColoredVertex, 3>>& triangles,
    size_t npoints_face,
    const FixedArray<float, 3>& lengths)
{
    auto res0 = generate_triangle_face_rays(triangles, npoints_face, lengths);
    auto res1 = generate_triangle_vertex_rays(triangles, lengths);
    res0.insert(res0.end(), res1.begin(), res1.end());
    return res0;
}
