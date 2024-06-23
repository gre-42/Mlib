#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TPos>
struct ColoredVertex;

/**
 * Source: https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
 */
bool ray_intersects_triangle(const FixedArray<double, 3>& ray_origin,
                             const FixedArray<double, 3>& ray_vector,
                             const FixedArray<FixedArray<double, 3>, 3>& triangle,
                             double t_max,
                             double& t,
                             FixedArray<double, 3>* intersection_point);

bool line_intersects_triangle(const FixedArray<double, 3>& ray_origin,
                              const FixedArray<double, 3>& ray_end,
                              const FixedArray<FixedArray<double, 3>, 3>& triangle,
                              double& t,
                              FixedArray<double, 3>* intersection_point);

bool line_intersects_triangle(const ColoredVertex<double>& ray_origin,
                              const ColoredVertex<double>& ray_end,
                              const FixedArray<ColoredVertex<double>, 3>& triangle,
                              double& t,
                              FixedArray<double, 3>* intersection_point);

}
