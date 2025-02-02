#include "Ray_Triangle_Intersection.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

using namespace Mlib;

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

/**
 * Source: https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
 */
bool Mlib::ray_intersects_triangle(const FixedArray<double, 3>& ray_origin,
                                   const FixedArray<double, 3>& ray_vector,
                                   const FixedArray<FixedArray<double, 3>, 3>& triangle,
                                   double t_max,
                                   double &t,
                                   FixedArray<double, 3>* intersection_point)
{
    const double EPSILON = 0.0000001f;
    const FixedArray<double, 3>& vertex0 = triangle(0);
    const FixedArray<double, 3>& vertex1 = triangle(1);
    const FixedArray<double, 3>& vertex2 = triangle(2);
    FixedArray<double, 3> edge1 = uninitialized;
    FixedArray<double, 3> edge2 = uninitialized;
    FixedArray<double, 3> h = uninitialized;
    FixedArray<double, 3> s = uninitialized;
    FixedArray<double, 3> q = uninitialized;
    double a, f, u, v;
    edge1 = vertex1 - vertex0;
    edge2 = vertex2 - vertex0;
    h = cross(ray_vector, edge2);
    a = dot0d(edge1, h);
    if (a > -EPSILON && a < EPSILON) {
        return false;    // This ray is parallel to this triangle.
    }
    f = 1.0 / a;
    s = ray_origin - vertex0;
    u = f * dot0d(s, h);
    if (u < 0.0 || u > 1.0) {
        return false;
    }
    q = cross(s, edge1);
    v = f * dot0d(ray_vector, q);
    if (v < 0.0 || u + v > 1.0) {
        return false;
    }
    // At this stage we can compute t to find out where the intersection point is on the line.
    t = f * dot0d(edge2, q);
    if (t > EPSILON && t < t_max) // ray intersection
    {
        if (intersection_point != nullptr) {
            *intersection_point = ray_origin + ray_vector * t;
        }
        return true;
    } else { // This means that there is a line intersection but not a ray intersection.
        return false;
    }
}

bool Mlib::line_intersects_triangle(const FixedArray<double, 3>& ray_origin,
                                    const FixedArray<double, 3>& ray_end,
                                    const FixedArray<FixedArray<double, 3>, 3>& triangle,
                                    double& t,
                                    FixedArray<double, 3>* intersection_point)
{
    auto ray_vector = ray_end - ray_origin;
    double t_max = std::sqrt(sum(squared(ray_vector)));
    ray_vector /= t_max;
    return ray_intersects_triangle(
        ray_origin,
        ray_vector,
        triangle,
        t_max,
        t,
        intersection_point);
}

bool Mlib::line_intersects_triangle(const ColoredVertex<double>& ray_origin,
                                    const ColoredVertex<double>& ray_end,
                                    const FixedArray<ColoredVertex<double>, 3>& triangle,
                                    double& t,
                                    FixedArray<double, 3>* intersection_point)
{
    return line_intersects_triangle(
        ray_origin.position,
        ray_end.position,
        FixedArray<FixedArray<double, 3>, 3>{
            triangle(0).position,
            triangle(1).position,
            triangle(2).position
        },
        t,
        intersection_point);
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
