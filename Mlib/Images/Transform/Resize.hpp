#pragma once
#include <Mlib/Images/Filters/Lowpass_Filter_Extension.hpp>
#include <cstddef>

namespace Mlib {

template <class TData>
class Array;
class ArrayShape;
enum class TargetShapeMode;

Array<float> resized_multichannel(
    const Array<float>& source,
    const ArrayShape& target_shape,
    TargetShapeMode target_shape_mode,
    FilterExtension fc = FilterExtension::NWE,
    float sigma = 0.1f);

Array<float> resized_singlechannel(
    const Array<float>& source,
    const ArrayShape& target_shape,
    TargetShapeMode target_shape_mode,
    FilterExtension fc = FilterExtension::NWE,
    float sigma = 0.1f);

Array<float> enlarged_multichannel_2d(const Array<float>& image, size_t factor);

Array<float> compressed_multichannel_2d(const Array<float>& image, size_t factor, FilterExtension fc);

}
