#pragma once
#include <Mlib/Images/Rgb24.hpp>

namespace Mlib {

template <class TData>
class Array;
template <typename TData, size_t... tshape>
class FixedArray;
class StbImage3;
class ArrayShape;

void hessian_determinant_trace(
    const Array<float>& image,
    Array<float>* determinant,
    Array<float>* trace);

Array<float> find_saddle_points(
    const Array<float>& image,
    float delta = -0.05f,
    float distance_sigma = 1.f);

Array<float> structure_tensor(
    const Array<float>& image,
    Array<float>* det = nullptr,
    Array<float>* trace = nullptr);

Array<float> harris_response(
    const Array<float>& image,
    float k = 0.05f);

Array<float> find_nfeatures(
    const Array<float>& featureness,
    const Array<bool>& mask,
    size_t nfeatures,
    float distance_sigma = 0);

void highlight_features(
    const Array<FixedArray<float, 2>>& feature_points,
    StbImage3& bitmap,
    size_t size = 1,
    const Rgb24& color = Rgb24::red());

void highlight_feature_correspondences(
    const Array<FixedArray<float, 2>>& feature_points0,
    const Array<FixedArray<float, 2>>& feature_points1,
    StbImage3& bitmap,
    size_t thickness = 0,
    const Rgb24& color = Rgb24::red(),
    const Rgb24* short_line_color = nullptr);

FixedArray<float, 2> find_feature_in_neighborhood(
    const FixedArray<float, 2>& old_feature_point,
    const Array<float>& new_featureness,
    const ArrayShape& window_shape);

}
