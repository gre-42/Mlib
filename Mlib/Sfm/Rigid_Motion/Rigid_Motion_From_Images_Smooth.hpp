#pragma once
#include <Mlib/Sfm/Homography/Homography_From_Images.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rigid_Motion_From_Images.hpp>

namespace Mlib { namespace Sfm { namespace Rmfi {

template <class TData>
Array<TData> rigid_motion_from_images_smooth(
    const Array<TData>& im_r,
    const Array<TData>& im_l,
    const Array<TData>& im_r_depth,
    const Array<TData>& intrinsic_matrix,
    const std::vector<TData>& sigmas,
    const Array<TData>& x0 = zeros<TData>(ArrayShape{6}))
{
    Array<TData> x00 = x0.copy();
    for(const TData& sigma : sigmas) {
        rigid_motion_from_images(
            gaussian_filter_NWE(im_r, sigma, NAN),
            gaussian_filter_NWE(im_l, sigma, NAN),
            gaussian_filter_NWE(im_r_depth, sigma, NAN),
            intrinsic_matrix,
            false,             // differentiate_numerically
            &x00,               // x0
            &x00);              // xe
    }
    return Cv::k_external(x00);
}

}}}
