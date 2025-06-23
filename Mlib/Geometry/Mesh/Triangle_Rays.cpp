#include "Triangle_Rays.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Mesh/Vertex_Normals.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Scene_Precision.hpp>

using namespace Mlib;

template <class TPos>
static FixedArray<TPos, 2, 3> gen_ray(
    const FixedArray<TPos, 3>& pos,
    const FixedArray<TPos, 3>& normal,
    const FixedArray<TPos, 3>& lengths)
{
    return {
        pos - sum(abs(lengths * normal)) / sum(abs(normal)) * normal,
        pos
    };
}

template <class TPos>
UUVector<FixedArray<TPos, 2, 3>> Mlib::generate_triangle_face_rays(
    const UUVector<FixedArray<ColoredVertex<TPos>, 3>>& triangles,
    size_t npoints,
    const FixedArray<TPos, 3>& lengths)
{
    using I = funpack_t<TPos>;
    UUVector<FixedArray<TPos, 2, 3>> res;
    res.reserve((npoints * (npoints + 1)) / 2 * triangles.size());
    size_t npoints2 = npoints + 2;
    for (const auto& t : triangles) {
        for (size_t u = 1; u < npoints2; ++u) {
            for (size_t v = 1; v < npoints2 - u; ++v) {
                if ((u + v) >= npoints2) {
                    break;
                }
                FixedArray<I, 3> bc{
                    I(u) / I(npoints2),
                    I(v) / I(npoints2),
                    I(npoints2 - u - v) / I(npoints2) };
                FixedArray<TPos, 3, 3> triangle{
                    t(0).position,
                    t(1).position,
                    t(2).position };
                auto pos = dot(bc, funpack(triangle));
                auto normal = triangle_normal(funpack(triangle));
                res.emplace_back(gen_ray(pos, normal, funpack(lengths)).template casted<TPos>());
            }
        }
    }
    return res;
}

template <class TPos>
UUVector<FixedArray<TPos, 2, 3>> Mlib::generate_triangle_vertex_rays(
    const UUVector<FixedArray<ColoredVertex<TPos>, 3>>& triangles,
    const FixedArray<TPos, 3>& lengths)
{
    VertexNormals<TPos, float> vertex_normals;
    vertex_normals.add_triangles(triangles.begin(), triangles.end());
    vertex_normals.compute_vertex_normals(NormalVectorErrorBehavior::THROW);

    UUVector<FixedArray<TPos, 2, 3>> res;
    res.reserve(vertex_normals.vertices().size());
    for (const auto& [v, n] : vertex_normals.vertices()) {
        using I = funpack_t<TPos>;
        res.emplace_back(gen_ray(funpack(v), n.template casted<I>(), funpack(lengths)).template casted<TPos>());
    }
    return res;
}

template <class TPos>
UUVector<FixedArray<TPos, 2, 3>> Mlib::generate_triangle_rays(
    const UUVector<FixedArray<ColoredVertex<TPos>, 3>>& triangles,
    size_t npoints_face,
    const FixedArray<TPos, 3>& lengths)
{
    auto res0 = generate_triangle_face_rays(triangles, npoints_face, lengths);
    auto res1 = generate_triangle_vertex_rays(triangles, lengths);
    res0.insert(res0.end(), res1.begin(), res1.end());
    return res0;
}

template UUVector<FixedArray<float, 2, 3>> Mlib::generate_triangle_face_rays(
    const UUVector<FixedArray<ColoredVertex<float>, 3>>& triangles,
    size_t npoints,
    const FixedArray<float, 3>& lengths);

template UUVector<FixedArray<float, 2, 3>> Mlib::generate_triangle_vertex_rays(
    const UUVector<FixedArray<ColoredVertex<float>, 3>>& triangles,
    const FixedArray<float, 3>& lengths);

template UUVector<FixedArray<float, 2, 3>> Mlib::generate_triangle_rays(
    const UUVector<FixedArray<ColoredVertex<float>, 3>>& triangles,
    size_t npoints_face,
    const FixedArray<float, 3>& lengths);

template UUVector<FixedArray<CompressedScenePos, 2, 3>> Mlib::generate_triangle_face_rays(
    const UUVector<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& triangles,
    size_t npoints,
    const FixedArray<CompressedScenePos, 3>& lengths);

template UUVector<FixedArray<CompressedScenePos, 2, 3>> Mlib::generate_triangle_vertex_rays(
    const UUVector<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& triangles,
    const FixedArray<CompressedScenePos, 3>& lengths);

template UUVector<FixedArray<CompressedScenePos, 2, 3>> Mlib::generate_triangle_rays(
    const UUVector<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& triangles,
    size_t npoints_face,
    const FixedArray<CompressedScenePos, 3>& lengths);
