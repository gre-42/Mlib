#pragma once
#include <Mlib/Geometry/Colored_Vertex.hpp>

namespace Mlib {

template <class TPos>
std::vector<FixedArray<FixedArray<TPos, 3>, 2>> generate_triangle_face_rays(
    const std::vector<FixedArray<ColoredVertex<TPos>, 3>>& triangles,
    size_t npoints,
    const FixedArray<TPos, 3>& lengths);

template <class TPos>
std::vector<FixedArray<FixedArray<TPos, 3>, 2>> generate_triangle_vertex_rays(
    const std::vector<FixedArray<ColoredVertex<TPos>, 3>>& triangles,
    const FixedArray<TPos, 3>& lengths);

template <class TPos>
std::vector<FixedArray<FixedArray<TPos, 3>, 2>> generate_triangle_rays(
    const std::vector<FixedArray<ColoredVertex<TPos>, 3>>& triangles,
    size_t npoints_face,
    const FixedArray<TPos, 3>& lengths);

}
