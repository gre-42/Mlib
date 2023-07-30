#include "Triangles_Around.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Intersection/Point_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>

using namespace Mlib;

std::list<const FixedArray<ColoredVertex<double>, 3>*> Mlib::get_triangles_around(
    const std::list<const FixedArray<ColoredVertex<double>, 3>*>& triangles,
    const FixedArray<double, 2>& pt,
    double radius)
{
    std::list<const FixedArray<ColoredVertex<double>, 3>*> tf;
    for (const auto& t : triangles) {
        FixedArray<double, 2> a{ (*t)(0).position(0), (*t)(0).position(1) };
        FixedArray<double, 2> b{ (*t)(1).position(0), (*t)(1).position(1) };
        FixedArray<double, 2> c{ (*t)(2).position(0), (*t)(2).position(1) };
        if (distance_point_to_triangle(pt, a, b, c) < radius) {
            tf.push_back(t);
        }
    }
    return tf;
}

std::list<const FixedArray<ColoredVertex<double>, 3>*> Mlib::get_triangles_around(
    const std::list<FixedArray<ColoredVertex<double>, 3>>& triangles,
    const FixedArray<double, 2>& pt,
    double radius)
{
    std::list<const FixedArray<ColoredVertex<double>, 3>*> tf;
    for (const auto& t : triangles) {
        tf.push_back(&t);
    }
    return get_triangles_around(tf, pt, radius);
}

std::list<const FixedArray<ColoredVertex<double>, 3>*> Mlib::get_triangles_around(
    const std::list<std::shared_ptr<TriangleList<double>>>& triangles,
    const FixedArray<double, 2>& pt,
    double radius)
{
    std::list<const FixedArray<ColoredVertex<double>, 3>*> tf;
    for (const auto& ts : triangles) {
        for (const auto& t : ts->triangles) {
            tf.push_back(&t);
        }
    }
    return get_triangles_around(tf, pt, radius);
}
