#pragma once
#include <cstddef>

namespace Mlib {

template <typename TData>
class Array;
template <typename TData, size_t... tshape>
class FixedArray;
class ArrayShape;
class StbImage3;

namespace Sfm{

void detect_chessboard(
    const Array<float>& image,
    const ArrayShape& shape,
    Array<FixedArray<float, 2>>& p_x,
    Array<FixedArray<float, 2>>& p_y,
    StbImage3& bmp);

}}
