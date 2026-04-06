#pragma once
#include <cstddef>

namespace Mlib {

template <class TData>
class Array;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
enum class AlphaChannelMode;

void draw_transformed(
    const Array<float>& image,
    Array<float>& canvas,
    const TransformationMatrix<float, float, 2>& trafo,
    AlphaChannelMode mode);

}
