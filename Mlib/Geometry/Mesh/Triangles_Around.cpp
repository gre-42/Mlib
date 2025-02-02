#include "Triangles_Around.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Point_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>

using namespace Mlib;

std::list<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*> Mlib::get_triangles_around(
    const std::list<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*>& triangles,
    const FixedArray<CompressedScenePos, 2>& pt,
    CompressedScenePos radius)
{
    std::list<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*> tf;
    for (const auto& t : triangles) {
        FixedArray<CompressedScenePos, 3, 2> tri{
            FixedArray<CompressedScenePos, 2>{ (*t)(0).position(0), (*t)(0).position(1) },
            FixedArray<CompressedScenePos, 2>{ (*t)(1).position(0), (*t)(1).position(1) },
            FixedArray<CompressedScenePos, 2>{ (*t)(2).position(0), (*t)(2).position(1) } };
        if ((CompressedScenePos)distance_point_to_triangle(funpack(pt), funpack(tri)) < radius) {
            tf.push_back(t);
        }
    }
    return tf;
}

std::list<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*> Mlib::get_triangles_around(
    const std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& triangles,
    const FixedArray<CompressedScenePos, 2>& pt,
    CompressedScenePos radius)
{
    std::list<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*> tf;
    for (const auto& t : triangles) {
        tf.push_back(&t);
    }
    return get_triangles_around(tf, pt, radius);
}

std::list<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*> Mlib::get_triangles_around(
    const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& triangles,
    const FixedArray<CompressedScenePos, 2>& pt,
    CompressedScenePos radius)
{
    std::list<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*> tf;
    for (const auto& ts : triangles) {
        for (const auto& t : ts->triangles) {
            tf.push_back(&t);
        }
    }
    return get_triangles_around(tf, pt, radius);
}
