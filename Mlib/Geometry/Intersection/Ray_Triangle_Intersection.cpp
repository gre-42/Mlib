#include "Ray_Triangle_Intersection.hpp"

using namespace Mlib;

#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize ("O3")
#endif

/**
 * Source: https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
 */
bool Mlib::ray_intersects_triangle(const FixedArray<float, 3>& ray_origin,
                                   const FixedArray<float, 3>& ray_vector,
                                   const FixedArray<FixedArray<float, 3>, 3>& triangle,
                                   FixedArray<float, 3>& intersection_point,
                                   float t_max)
{
    const float EPSILON = 0.0000001f;
    const FixedArray<float, 3>& vertex0 = triangle(0);
    const FixedArray<float, 3>& vertex1 = triangle(1);
    const FixedArray<float, 3>& vertex2 = triangle(2);
    FixedArray<float, 3> edge1, edge2, h, s, q;
    float a, f, u, v;
    edge1 = vertex1 - vertex0;
    edge2 = vertex2 - vertex0;
    h = cross(ray_vector, edge2);
    a = dot0d(edge1, h);
    if (a > -EPSILON && a < EPSILON) {
        return false;    // This ray is parallel to this triangle.
    }
    f = 1.f / a;
    s = ray_origin - vertex0;
    u = f * dot0d(s, h);
    if (u < 0.f || u > 1.f) {
        return false;
    }
    q = cross(s, edge1);
    v = f * dot0d(ray_vector, q);
    if (v < 0.f || u + v > 1.f) {
        return false;
    }
    // At this stage we can compute t to find out where the intersection point is on the line.
    float t = f * dot0d(edge2, q);
    if (t > EPSILON && t < t_max) // ray intersection
    {
        intersection_point = ray_origin + ray_vector * t;
        return true;
    } else { // This means that there is a line intersection but not a ray intersection.
        return false;
    }
}

bool Mlib::line_intersects_triangle(const FixedArray<float, 3>& ray_origin,
                                    const FixedArray<float, 3>& ray_end,
                                    const FixedArray<FixedArray<float, 3>, 3>& triangle,
                                    FixedArray<float, 3>& intersection_point)
{
    auto ray_vector = ray_end - ray_origin;
    float t_max = std::sqrt(sum(squared(ray_vector)));
    ray_vector /= t_max;
    return ray_intersects_triangle(
        ray_origin,
        ray_vector,
        triangle,
        intersection_point,
        t_max);
}

bool Mlib::line_intersects_triangle(const ColoredVertex& ray_origin,
                                    const ColoredVertex& ray_end,
                                    const FixedArray<ColoredVertex, 3>& triangle,
                                    FixedArray<float, 3>& intersection_point)
{
    return line_intersects_triangle(
        ray_origin.position,
        ray_end.position,
        FixedArray<FixedArray<float, 3>, 3>{
            triangle(0).position,
            triangle(1).position,
            triangle(2).position
        },
        intersection_point);
}

#ifdef __GNUC__
    #pragma GCC pop_options
#endif
