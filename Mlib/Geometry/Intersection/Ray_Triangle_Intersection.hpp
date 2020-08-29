#pragma once
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Math/Fixed_Math.hpp>

namespace Mlib {

/**
 * Source: https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
 */
bool ray_intersects_triangle(const FixedArray<float, 3>& ray_origin,
                             const FixedArray<float, 3>& ray_vector,
                             const FixedArray<FixedArray<float, 3>, 3>& triangle,
                             FixedArray<float, 3>& intersection_point,
                             float t_max = INFINITY);

bool line_intersects_triangle(const FixedArray<float, 3>& ray_origin,
                              const FixedArray<float, 3>& ray_end,
                              const FixedArray<FixedArray<float, 3>, 3>& triangle,
                              FixedArray<float, 3>& intersection_point);

bool line_intersects_triangle(const ColoredVertex& ray_origin,
                              const ColoredVertex& ray_end,
                              const FixedArray<ColoredVertex, 3>& triangle,
                              FixedArray<float, 3>& intersection_point);

}
