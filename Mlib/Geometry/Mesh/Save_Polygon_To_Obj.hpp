#pragma once
#include <cstddef>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

void save_polygon_to_obj(
    const std::string& filename,
    const FixedArray<double, 2, 3>& edge);

void save_polygon_to_obj(
    const std::string& filename,
    const FixedArray<double, 3, 3>& triangle);

void save_polygon_to_obj(
    const std::string& filename,
    const FixedArray<double, 4, 3>& quad);

}
