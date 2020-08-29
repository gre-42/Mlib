#pragma once
#include <Mlib/Geometry/Colored_Vertex.hpp>

namespace Mlib {

std::vector<FixedArray<FixedArray<float, 3>, 2>> generate_triangle_face_rays(
    const std::vector<FixedArray<ColoredVertex, 3>>& triangles,
    size_t npoints,
    const FixedArray<float, 3>& lengths);

std::vector<FixedArray<FixedArray<float, 3>, 2>> generate_triangle_vertex_rays(
    const std::vector<FixedArray<ColoredVertex, 3>>& triangles,
    const FixedArray<float, 3>& lengths);

std::vector<FixedArray<FixedArray<float, 3>, 2>> generate_triangle_rays(
    const std::vector<FixedArray<ColoredVertex, 3>>& triangles,
    size_t npoints_face,
    const FixedArray<float, 3>& lengths);

}
