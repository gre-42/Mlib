#pragma once
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Math/Rodrigues.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rigid_Motion_From_Images.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rotation_From_Images.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <optional>

namespace Mlib::Sfm::Rmfi {

/**
 * This function returns the projection: l <- r1 <- r0
 * l: live image
 * r1: reference 1 w/o depth, but close to l
 * r0: reference 0 including depth (keyframe)
 *
 * x0_r1_r0: projection from r0 to r1
 */
template <class TData>
TransformationMatrix<float, float, 3> rigid_motion_from_images_robust(
    const Array<TData>& im_r0,
    const Array<TData>& im_r1,
    const Array<TData>& im_l,
    const Array<TData>& im_r0_depth,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix_r0,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix_r1,
    const TransformationMatrix<float, float, 2>& intrinsic_matrix_l,
    const Array<TData>& sigmas,
    const Array<TData>& thresholds,
    const FixedArray<TData, 6>& x0_r1_r0 = fixed_zeros<TData, 6>(),
    bool estimate_rotation_first = true,
    bool print_residual = true)
{
    assert(im_r0.ndim() == 3);
    assert(all(im_r0.shape() == im_r1.shape()));
    assert(all(im_r0.shape() == im_l.shape()));
    assert(all(im_r0.shape().erased_first() == im_r0_depth.shape()));
    FixedArray<TData, 3> x0_rot_l_r1 = fixed_zeros<TData, 3>();
    if (estimate_rotation_first) {
        for (const TData& sigma : sigmas.flat_iterable()) {
            Hfi::rotation_from_images(
                multichannel_gaussian_filter_NWE(im_r1, sigma, NAN),
                multichannel_gaussian_filter_NWE(im_l, sigma, NAN),
                intrinsic_matrix_r1,
                intrinsic_matrix_l,
                false,
                &x0_rot_l_r1,
                &x0_rot_l_r1,
                print_residual);
        }
    }
    FixedArray<TData, 6> x0_l_r1 = fixed_zeros<TData, 6>();
    x0_l_r1.template row_range<0, 3>() = x0_rot_l_r1;

    //Array<TData> x0_l_r0 = x0_r1_r0.copy();
    //x0_l_r0.row_range(0, 3) = x0_rot_l_r1;
    FixedArray<TData, 6> x0_l_r0 = Cv::k_external_inverse(
        Cv::k_external(x0_l_r1) *
        Cv::k_external(x0_r1_r0));

    assert(thresholds.length() == sigmas.length() - 1);
    auto threshold_it = thresholds.flat_begin();
    std::optional<TransformationMatrix<TData, TData, 3>> ke;
    for (const TData& sigma : sigmas.flat_iterable()) {
        Array<TData> masked_im_r_depth_s = gaussian_filter_NWE(im_r0_depth, sigma, NAN);
        if (ke.has_value()) {
            // Assign NANs to pixels with errors above a given threshold.
            Array<float> err = mean(
                abs(d_pr_bilinear(im_r0, im_l, im_r0_depth, intrinsic_matrix_r0, intrinsic_matrix_l, *ke)),
                0);
            for (size_t r = 0; r < im_r0.shape(1); ++r) {
                for (size_t c = 0; c < im_r0.shape(2); ++c) {
                    if (std::isnan(err(r, c)) || (err(r, c) >= (*threshold_it))) {
                        masked_im_r_depth_s(r, c) = NAN;
                    }
                }
            }
            // draw_nan_masked_grayscale(masked_im_r_depth_s, 0, 0).save_to_file("masked_im_r_depth_s-" + std::to_string(*threshold_it) + ".ppm");
            ++threshold_it;
        }
        ke = rigid_motion_from_images(
            multichannel_gaussian_filter_NWE(im_r0, sigma, NAN),
            multichannel_gaussian_filter_NWE(im_l, sigma, NAN),
            masked_im_r_depth_s,
            intrinsic_matrix_r0,
            intrinsic_matrix_l,
            false,             // differentiate_numerically
            &x0_l_r0,          // x0
            &x0_l_r0,          // xe
            print_residual);   // print_residual
    }
    return TransformationMatrix<float, float, 3>{ Cv::k_external(x0_l_r0) };
}

}
