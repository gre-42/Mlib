#include "Triangles_Around.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Point_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>

using namespace Mlib;

std::list<const FixedArray<ColoredVertex, 3>*> Mlib::get_triangles_around(
    const std::list<const FixedArray<ColoredVertex, 3>*>& triangles,
    const FixedArray<float, 2>& pt,
    float radius)
{
    std::list<const FixedArray<ColoredVertex, 3>*> tf;
    for (const auto& t : triangles) {
        FixedArray<float, 2> a{ (*t)(0).position(0), (*t)(0).position(1) };
        FixedArray<float, 2> b{ (*t)(1).position(0), (*t)(1).position(1) };
        FixedArray<float, 2> c{ (*t)(2).position(0), (*t)(2).position(1) };
        if (distance_point_to_triangle(pt, a, b, c) < radius) {
            tf.push_back(t);
        }
    }
    return tf;
}

std::list<const FixedArray<ColoredVertex, 3>*> Mlib::get_triangles_around(
    const std::list<FixedArray<ColoredVertex, 3>>& triangles,
    const FixedArray<float, 2>& pt,
    float radius)
{
    std::list<const FixedArray<ColoredVertex, 3>*> tf;
    for (const auto& t : triangles) {
        tf.push_back(&t);
    }
    return get_triangles_around(tf, pt, radius);
}

std::list<const FixedArray<ColoredVertex, 3>*> Mlib::get_triangles_around(
    const std::list<std::shared_ptr<TriangleList>>& triangles,
    const FixedArray<float, 2>& pt,
    float radius)
{
    std::list<const FixedArray<ColoredVertex, 3>*> tf;
    for (const auto& ts : triangles) {
        for (const auto& t : ts->triangles_) {
            tf.push_back(&t);
        }
    }
    return get_triangles_around(tf, pt, radius);
}
