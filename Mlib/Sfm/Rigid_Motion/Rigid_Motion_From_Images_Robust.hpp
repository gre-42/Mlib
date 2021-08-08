#pragma once
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Math/Rodrigues.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rigid_Motion_From_Images.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rotation_From_Images.hpp>

namespace Mlib::Sfm::Rmfi {

/**
 * Projection: l <- r1 <- r0
 * l: live image
 * r1: reference 1 w/o depth, but close to l
 * r0: reference 0 including depth (keyframe)
 *
 * x0_r1_r0: projection from r0 to r1
 */
template <class TData>
TransformationMatrix<float, 3> rigid_motion_from_images_robust(
    const Array<TData>& im_r0,
    const Array<TData>& im_r1,
    const Array<TData>& im_l,
    const Array<TData>& im_r0_depth,
    const TransformationMatrix<float, 2>& intrinsic_matrix,
    const std::vector<TData>& sigmas,
    const std::vector<TData>& thresholds,
    const FixedArray<TData, 6>& x0_r1_r0 = fixed_zeros<TData, 6>(),
    bool estimate_rotation_first = true,
    bool print_residual = true)
{
    FixedArray<TData, 3> x0_rot_l_r1 = fixed_zeros<TData, 3>();
    if (estimate_rotation_first) {
        for (const TData& sigma : sigmas) {
            Hfi::rotation_from_images(
                gaussian_filter_NWE(im_r1, sigma, NAN),
                gaussian_filter_NWE(im_l, sigma, NAN),
                intrinsic_matrix,
                false,
                &x0_rot_l_r1,
                &x0_rot_l_r1,
                print_residual);
        }
    }
    FixedArray<TData, 6> x0_l_r1 = fixed_zeros<TData, 6>();
    x0_l_r1 TEMPLATEV row_range<0, 3>() = x0_rot_l_r1;

    //Array<TData> x0_l_r0 = x0_r1_r0.copy();
    //x0_l_r0.row_range(0, 3) = x0_rot_l_r1;
    FixedArray<TData, 6> x0_l_r0 = Cv::k_external_inverse(
        Cv::k_external(x0_l_r1) *
        Cv::k_external(x0_r1_r0));

    assert(thresholds.size() == sigmas.size() - 1);
    auto threshold_it = thresholds.begin();
    bool ke_initialized = false;
    TransformationMatrix<TData, 3> ke;
    for (const TData& sigma : sigmas) {
        Array<TData> masked_im_r_depth_s = gaussian_filter_NWE(im_r0_depth, sigma, NAN);
        if (ke_initialized) {
            masked_im_r_depth_s = masked_im_r_depth_s.array_array_binop(
                d_pr_bilinear(im_r0, im_l, im_r0_depth, intrinsic_matrix, ke),
                [&threshold_it](const TData& depth, const TData& err){ return (!std::isnan(err)) && (std::abs(err) < *threshold_it) ? depth : NAN; });
            // draw_nan_masked_grayscale(masked_im_r_depth_s, 0, 0).save_to_file("masked_im_r_depth_s-" + std::to_string(*threshold_it) + ".ppm");
            ++threshold_it;
        }
        ke = rigid_motion_from_images(
            gaussian_filter_NWE(im_r0, sigma, NAN),
            gaussian_filter_NWE(im_l, sigma, NAN),
            masked_im_r_depth_s,
            intrinsic_matrix,
            false,             // differentiate_numerically
            &x0_l_r0,          // x0
            &x0_l_r0,          // xe
            print_residual);   // print_residual
        ke_initialized = true;
    }
    return TransformationMatrix<float, 3>{ Cv::k_external(x0_l_r0) };
}

}
