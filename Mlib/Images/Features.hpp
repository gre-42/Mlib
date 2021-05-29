#pragma once
#include <Mlib/Images/Rgb24.hpp>

namespace Mlib {

template <class TData>
class Array;
class StbImage;
class ArrayShape;

void hessian_determinant_trace(
    const Array<float>& image,
    Array<float>* determinant,
    Array<float>* trace);

Array<float> find_saddle_points(
    const Array<float>& image,
    float delta = -0.05);

Array<float> structure_tensor(
    const Array<float>& image,
    Array<float>* det = nullptr,
    Array<float>* trace = nullptr);

Array<float> harris_response(
    const Array<float>& image,
    Array<bool>* feature_mask = nullptr);

Array<float> find_harris_corners(
    const Array<float>& image,
    float threshold = -0.01);

Array<float> find_nfeatures(
    const Array<float>& featureness,
    const Array<bool>& mask,
    size_t nfeatures,
    float distance_sigma = 0);

void highlight_features(
    const Array<float>& feature_points,
    StbImage& bitmap,
    size_t size = 1,
    const Rgb24& color = Rgb24::red());

Array<float> find_feature_in_neighborhood(
    const Array<float>& old_feature_point,
    const Array<float>& new_featureness,
    const ArrayShape& window_shape);

}
