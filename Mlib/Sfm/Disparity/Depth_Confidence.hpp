#pragma once
#include <cstddef>

namespace Mlib {
   
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TData>
class Array;
template <typename TData, size_t... tshape>
class FixedArray;

namespace Sfm {

/**
 * From: Real-Time Visibility-Based Fusion of Depth Maps.
 * IEEE International Conference on Computer Vision, 2007.
 */
Array<float> depth_confidence(const Array<float>& cost_volume);

}

}
