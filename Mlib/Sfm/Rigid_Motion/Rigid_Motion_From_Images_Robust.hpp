#pragma once
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Math/Rodrigues.hpp>
#include <Mlib/Sfm/Homography/Homography_From_Images.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rigid_Motion_From_Images.hpp>

namespace Mlib { namespace Sfm { namespace Rmfi {

/**
 * Projection: l <- r1 <- r0
 * l: live image
 * r1: reference 1 w/o depth, but close to l
 * r0: reference 0 including depth (keyframe)
 *
 * x0_r1_r0: projection from r0 to r1
 */
template <class TData>
Array<TData> rigid_motion_from_images_robust(
    const Array<TData>& im_r0,
    const Array<TData>& im_r1,
    const Array<TData>& im_l,
    const Array<TData>& im_r0_depth,
    const Array<TData>& intrinsic_matrix,
    const std::vector<TData>& sigmas,
    const std::vector<TData>& thresholds,
    const Array<TData>& x0_r1_r0 = zeros<TData>(ArrayShape{6}),
    bool estimate_rotation_first = true)
{
    Array<TData> x0_rot_l_r1 = zeros<TData>(ArrayShape{3});
    if (estimate_rotation_first) {
        for (const TData& sigma : sigmas) {
            Hfi::rotation_from_images(
                gaussian_filter_NWE(im_r1, sigma, NAN),
                gaussian_filter_NWE(im_l, sigma, NAN),
                intrinsic_matrix,
                false,
                &x0_rot_l_r1,
                &x0_rot_l_r1);
        }
    }
    Array<TData> x0_l_r1 = zeros<TData>(ArrayShape{6});
    x0_l_r1.row_range(0, 3) = x0_rot_l_r1;

    //Array<TData> x0_l_r0 = x0_r1_r0.copy();
    //x0_l_r0.row_range(0, 3) = x0_rot_l_r1;
    Array<TData> x0_l_r0 = Cv::k_external_inverse(dot(
        homogenized_4x4(Cv::k_external(x0_l_r1)),
        homogenized_4x4(Cv::k_external(x0_r1_r0))).row_range(0, 3));

    assert(thresholds.size() == sigmas.size() - 1);
    auto threshold_it = thresholds.begin();
    Array<TData> ke;
    for (const TData& sigma : sigmas) {
        Array<TData> masked_im_r_depth_s = gaussian_filter_NWE(im_r0_depth, sigma, NAN);
        if (ke.initialized()) {
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
            &x0_l_r0);         // xe
    }
    return Cv::k_external(x0_l_r0);
}

}}}
