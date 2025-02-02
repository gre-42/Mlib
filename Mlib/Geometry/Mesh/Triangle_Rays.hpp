#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TPos>
struct ColoredVertex;

template <class TPos>
UUVector<FixedArray<TPos, 2, 3>> generate_triangle_face_rays(
    const UUVector<FixedArray<ColoredVertex<TPos>, 3>>& triangles,
    size_t npoints,
    const FixedArray<TPos, 3>& lengths);

template <class TPos>
UUVector<FixedArray<TPos, 2, 3>> generate_triangle_vertex_rays(
    const UUVector<FixedArray<ColoredVertex<TPos>, 3>>& triangles,
    const FixedArray<TPos, 3>& lengths);

template <class TPos>
UUVector<FixedArray<TPos, 2, 3>> generate_triangle_rays(
    const UUVector<FixedArray<ColoredVertex<TPos>, 3>>& triangles,
    size_t npoints_face,
    const FixedArray<TPos, 3>& lengths);

}
