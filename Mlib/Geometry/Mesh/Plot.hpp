#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Images/PpmImage.hpp>

namespace Mlib {

class ColoredVertex;

PpmImage plot_mesh(
    const ArrayShape& image_size,
    const std::list<FixedArray<FixedArray<float, 2>, 3>>& triangles,
    const std::list<FixedArray<float, 2>>& contour,
    const std::list<FixedArray<float, 2>>& highlighted_nodes);

PpmImage plot_mesh(
    const ArrayShape& image_size,
    const std::list<FixedArray<ColoredVertex, 3>>& triangles,
    const std::list<FixedArray<float, 3>>& contour,
    const std::list<FixedArray<float, 3>>& highlighted_nodes);

}
