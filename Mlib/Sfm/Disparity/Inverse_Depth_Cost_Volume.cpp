#include "Inverse_Depth_Cost_Volume.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Sfm/Homography/Apply_Homography.hpp>
#include <Mlib/Sfm/Homography/Homography_From_Transform.hpp>
#include <Mlib/Sfm/Homography/Homography_Sampler.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>
#include <Mlib/Stats/Min_Max.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

InverseDepthCostVolume::InverseDepthCostVolume(
    const ArrayShape& space_shape,
    const Array<float>& inverse_depths)
: space_shape_{space_shape},
  inverse_depths_{inverse_depths},
  idsi_sum_{zeros<float>(ArrayShape{inverse_depths.length()}.concatenated(space_shape))},
  nelements_idsi_{zeros<size_t>(idsi_sum_.shape())},
  nchannel_increments_(0)
{
    assert(inverse_depths.ndim() == 1);
}

void InverseDepthCostVolume::increment(
    const Array<float>& intrinsic_matrix,
    const Array<float>& ke0,
    const Array<float>& ke1,
    const Array<float>& im0_rgb,
    const Array<float>& im1_rgb,
    const float epipole_radius)
{
    assert(all(intrinsic_matrix.shape() == ArrayShape{3, 3}));
    assert(all(ke0.shape() == ArrayShape{3, 4}));
    assert(all(ke1.shape() == ArrayShape{3, 4}));
    assert(im0_rgb.ndim() == 3);
    assert(all(im0_rgb.shape() == im1_rgb.shape()));
    assert(all(im0_rgb.shape().erased_first() == space_shape_));

    nchannel_increments_ += im0_rgb.shape(0);

    Array<float> ke = projection_in_reference(ke0, ke1);
    Array<float> t = t3_from_Nx4(ke, 3);
    Array<float> R = R3_from_Nx4(ke, 3);
    Array<float> e;
    if (epipole_radius != 0) {
        if (max(abs(ke - identity_array<float>(4).row_range(0, 3))) > 1e-3) {
            Array<float> e3 = find_epipole(intrinsic_matrix, ke);
            if (e3(2) != 0) {
                e = a2fi(dehomogenized_2(e3));
            }
        }
    }

    for (size_t di = 0; di < inverse_depths_.length(); ++di) {
        Array<float> homog_e = rotation_and_translation_to_homography(
            R,
            t,
            -Array<float>{0, 0, 1},
            1 / inverse_depths_(di));
        Array<float> homog_i = pixel_homography(intrinsic_matrix, homog_e);
        FixedArray<float, 3, 3> homog_i_f{homog_i};
        FixedArray<size_t, 2> space_shape_f{space_shape_};
        HomographySampler<float> hs{homog_i};
        #pragma omp parallel for
        for (int ri = 0; ri < (int)im0_rgb.shape(1); ++ri) {
            size_t r = (size_t)ri;
            for (size_t c = 0; c < im0_rgb.shape(2); ++c) {
                if (e.initialized() && (std::abs(r - e(0)) < 80) && (std::abs(c - e(1)) < 80)) {
                    continue;
                }
                if (false) {
                    ArrayShape i0{r, c};
                    Array<float> ai0 = i2a(i0);
                    Array<float> ai1 = apply_homography(homog_i, homogenized_3(ai0));
                    ArrayShape i1 = a2i(dehomogenized_2(ai1));
                    // std::cerr << "i0 " << i0 << " i1 " << i1 << std::endl;
                    if (all(i1 < space_shape_)) {
                        for (size_t h = 0; h < im0_rgb.shape(0); ++h) {
                            idsi_sum_(di, r, c) += std::abs(
                                im0_rgb(h, i0(0), i0(1)) -
                                im1_rgb(h, i1(0), i1(1)));
                            ++nelements_idsi_(di, r, c);
                        }
                    }
                } else if (false) {
                    FixedArray<size_t, 2> i0{r, c};
                    FixedArray<float, 2> ai0 = i2a(i0);
                    FixedArray<float, 3> ai1 = apply_homography(homog_i_f, homogenized_3(ai0));
                    FixedArray<size_t, 2> i1 = a2i(dehomogenized_2(ai1));
                    if (all(i1 < space_shape_f)) {
                        for (size_t h = 0; h < im0_rgb.shape(0); ++h) {
                            idsi_sum_(di, r, c) += std::abs(
                                im0_rgb(h, i0(0), i0(1)) -
                                im1_rgb(h, i1(0), i1(1)));
                            ++nelements_idsi_(di, r, c);
                        }
                    }
                } else {
                    BilinearInterpolator<float> bi;
                    if (hs.sample_destination(r, c, space_shape_, bi)) {
                        for (size_t h = 0; h < im0_rgb.shape(0); ++h) {
                            float v = bi.interpolate_multichannel(im1_rgb, h);
                            if (!std::isnan(v)) {
                                idsi_sum_(di, r, c) += std::abs(im0_rgb(h, r, c) - v);
                                ++nelements_idsi_(di, r, c);
                            }
                        }
                    }
                }
            }
        }
    }
}

Array<float> InverseDepthCostVolume::get(size_t min_channel_increments) const {
    if (nchannel_increments_ < min_channel_increments) {
        throw std::runtime_error("nincrements is smaller than min_channel_increments");
    }
    Array<float> res = idsi_sum_.array_array_binop(nelements_idsi_, [&](float s, size_t n) {
        return n == 0 ? NAN : s / n;
    });
    Array<bool> all_set = all(nelements_idsi_ >= size_t(min_channel_increments), 0);
    for (size_t di = 0; di < inverse_depths_.length(); ++di) {
        for (size_t r = 0; r < res.shape(1); ++r) {
            for (size_t c = 0; c < res.shape(2); ++c) {
                if (!all_set(r, c)) {
                    res(di, r, c) = NAN;
                }
            }
        }
    }
    return res;
}
