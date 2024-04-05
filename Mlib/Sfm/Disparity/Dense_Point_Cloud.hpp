#pragma once
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {
   
template <class TDir, class TPos, size_t n>
class TransformationMatrix;

namespace Sfm {

Array<float> harris_response_1d(
    const Array<float>& im1,
    const FixedArray<float, 3, 3>& F);

/**
 * Cf. "CorrespondingFeaturesOnLine"
 */
Array<float> compute_disparity_gray_single_pixel(
    const Array<float>& im0,
    const Array<float>& im1,
    const FixedArray<float, 3, 3>& F,
    size_t search_length,
    bool l1_normalization = true,
    Array<float>* dsi = nullptr,
    float d_multiplier = 1);

Array<float> compute_disparity_rgb_single_pixel(
    const Array<float>& im0,
    const Array<float>& im1,
    const FixedArray<float, 3, 3>& F,
    size_t search_length,
    bool l1_normalization = true,
    Array<float>* dsi = nullptr,
    float d_multiplier = 1);

Array<float> compute_disparity_rgb_patch(
    const Array<float>& im0_rgb,
    const Array<float>& im1_rgb,
    const FixedArray<float, 3, 3>& F,
    size_t search_length,
    float worst_error,
    const FixedArray<size_t, 2>& patch_size = FixedArray<size_t, 2>{ 10u, 10u },
    const FixedArray<size_t, 2>& patch_nan_size = FixedArray<size_t, 2>{ 0u, 0u },
    Array<float>* im0_error = nullptr,
    Array<bool>* mask = nullptr,
    Array<float>* prior_disparity = nullptr,
    Array<float>* prior_strength = nullptr);

Array<float> iterate_disparity_rgb_patch(
    const Array<float>& im0_gray,
    const Array<float>& im0_rgb,
    const Array<float>& im1_rgb,
    const FixedArray<float, 3, 3>& F,
    size_t search_length,
    float worst_error,
    const FixedArray<size_t, 2>& patch_size = FixedArray<size_t, 2>{ 10u, 10u },
    const FixedArray<size_t, 2>& patch_nan_size = FixedArray<size_t, 2>{ 0u, 0u },
    Array<float>* im0_error = nullptr,
    Array<float>* initial_disparity = nullptr,
    bool draw_debug_images = false);

Array<float> move_along_disparity(
    const Array<float>& disparity,
    const FixedArray<float, 3, 3>& F,
    const Array<float>& img1_rgb,
    const Array<bool>* mask = nullptr,
    Array<bool>* reverse_mask = nullptr);

Array<float> reconstruct_disparity(
    const Array<float>& disparity,
    const FixedArray<float, 3, 3>& F,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    const TransformationMatrix<float, float, 3>& extrinsic_matrix,
    Array<float>* condition_number = nullptr);

Array<float> reconstruct_depth(
    const Array<float>& depth,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    const TransformationMatrix<float, float, 3>& extrinsic_matrix);

}}
