#pragma once
#include <cstddef>

namespace Mlib {

struct IntersectionScene;
template <typename TData, size_t... tshape>
class FixedArray;

void handle_line_triangle_intersection(const IntersectionScene& c);
void handle_line_triangle_intersection(
    const IntersectionScene& c,
    const FixedArray<double, 3>& intersection_point);

}
