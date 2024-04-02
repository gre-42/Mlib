#pragma once
#include <cstddef>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

void save_triangle_to_obj(
    const std::string& filename,
    const FixedArray<FixedArray<double, 3>, 3>& triangle);

void save_quad_to_obj(
    const std::string& filename,
    const FixedArray<FixedArray<double, 3>, 4>& quad);

}
