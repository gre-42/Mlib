#pragma once
#include <cstddef>

namespace Mlib {

template <class TData>
class Array;
template <typename TData, size_t... tshape>
class FixedArray;

Array<FixedArray<FixedArray<float, 3>, 3>> flip_edges_3d(
    const Array<FixedArray<FixedArray<float, 3>, 3>>& mesh);

}
