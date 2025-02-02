#pragma once
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rigid_Motion_From_Images.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rotation_From_Images.hpp>

namespace Mlib::Sfm::Rmfi {

template <class TData>
TransformationMatrix<TData, TData, 3> rigid_motion_from_images_smooth(
    const Array<TData>& im_r,
    const Array<TData>& im_l,
    const Array<TData>& im_r_depth,
    const TransformationMatrix<TData, TData, 2>& intrinsic_matrix_r,
    const TransformationMatrix<TData, TData, 2>& intrinsic_matrix_l,
    const std::vector<TData>& sigmas,
    const FixedArray<TData, 6>& x0 = fixed_zeros<TData, 6>(),
    bool print_residual = true)
{
    FixedArray<TData, 6> x00 = x0;
    for (const TData& sigma : sigmas) {
        rigid_motion_from_images(
            multichannel_gaussian_filter_NWE(im_r, sigma, NAN),
            multichannel_gaussian_filter_NWE(im_l, sigma, NAN),
            gaussian_filter_NWE(im_r_depth, sigma, NAN),
            intrinsic_matrix_r,
            intrinsic_matrix_l,
            false,              // differentiate_numerically
            &x00,               // x0
            &x00,               // xe
            print_residual);
    }
    return Cv::k_external(x00);
}

}
