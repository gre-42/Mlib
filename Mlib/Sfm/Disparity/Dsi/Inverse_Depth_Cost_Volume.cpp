#include "Inverse_Depth_Cost_Volume.hpp"
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Images/Resample/Down_Sample_Average.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Sfm/Homography/Homography_From_Transform.hpp>
#include <Mlib/Sfm/Homography/Homography_Sampler.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>
#include <Mlib/Stats/Min_Max.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

InverseDepthCostVolume::InverseDepthCostVolume(const Array<float>& dsi)
: dsi_(dsi)
{}

Array<float> InverseDepthCostVolume::dsi() const {
    return dsi_;
}

std::unique_ptr<CostVolume> InverseDepthCostVolume::down_sampled() const {
    return std::make_unique<InverseDepthCostVolume>(multichannel_down_sample_average(dsi_));
}

size_t InverseDepthCostVolume::nlayers() const {
    return dsi_.shape(0);
}

InverseDepthCostVolumeAccumulator::InverseDepthCostVolumeAccumulator(
    const ArrayShape& space_shape,
    const Array<float>& inverse_depths)
: space_shape_(space_shape),
  inverse_depths_(inverse_depths),
  idsi_sum_(zeros<float>(ArrayShape{inverse_depths.length()}.concatenated(space_shape))),
  nelements_idsi_(zeros<size_t>(idsi_sum_.shape())),
  nchannel_increments_(0)
{
    assert(inverse_depths.ndim() == 1);
}

void InverseDepthCostVolumeAccumulator::increment(
    const TransformationMatrix<float, float, 2>& intrinsic_matrix,
    const TransformationMatrix<float, float, 3>& ke0,
    const TransformationMatrix<float, float, 3>& ke1,
    const Array<float>& im0_rgb,
    const Array<float>& im1_rgb,
    const float epipole_radius)
{
    assert(im0_rgb.ndim() == 3);
    assert(all(im0_rgb.shape() == im1_rgb.shape()));
    assert(all(im0_rgb.shape().erased_first() == space_shape_));

    nchannel_increments_ += im0_rgb.shape(0);

    TransformationMatrix<float, float, 3> ke = projection_in_reference(ke0, ke1);
    FixedArray<float, 2> e{ NAN };
    if (epipole_radius != 0) {
        if (max(abs(ke.affine() - fixed_identity_array<float, 4>())) > 1e-3) {
            FixedArray<float, 2> e2 = find_epipole(intrinsic_matrix, ke);
            if (!all(Mlib::isnan(e2))) {
                e = a2fi(e2);
            }
        }
    }

    for (size_t di = 0; di < inverse_depths_.length(); ++di) {
        FixedArray<float, 3, 3> homog_e = rotation_and_translation_to_homography(
            ke,
            -FixedArray<float, 3>{0.f, 0.f, 1.f},
            1.f / inverse_depths_(di));
        FixedArray<float, 3, 3> homog_i = pixel_homography(intrinsic_matrix, homog_e);
        FixedArray<size_t, 2> space_shape_f{space_shape_};
        HomographySampler<float> hs{homog_i};
        #pragma omp parallel for
        for (int ri = 0; ri < (int)im0_rgb.shape(1); ++ri) {
            size_t r = (size_t)ri;
            for (size_t c = 0; c < im0_rgb.shape(2); ++c) {
                if ((epipole_radius != 0) && !any(Mlib::isnan(e)) && (std::abs(r - e(0)) < epipole_radius) && (std::abs(c - e(1)) < epipole_radius)) {
                    continue;
                }
                if (false) {
                    FixedArray<size_t, 2> i0{r, c};
                    FixedArray<float, 2> ai0 = i2a(i0);
                    FixedArray<float, 2> ai1 = apply_homography(homog_i, ai0);
                    FixedArray<size_t, 2> i1 = a2i(ai1);
                    // lerr() << "i0 " << i0 << " i1 " << i1;
                    if (all(i1 < space_shape_f)) {
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
                    FixedArray<float, 2> ai1 = apply_homography(homog_i, ai0);
                    FixedArray<size_t, 2> i1 = a2i(ai1);
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
                    if (hs.sample_destination(r, c, space_shape_(0), space_shape_(1), bi)) {
                        for (size_t h = 0; h < im0_rgb.shape(0); ++h) {
                            float v = bi(im1_rgb, h);
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

std::unique_ptr<CostVolume> InverseDepthCostVolumeAccumulator::get(size_t min_channel_increments) const {
    if (nchannel_increments_ < min_channel_increments) {
        throw std::runtime_error("nincrements is smaller than min_channel_increments");
    }
    Array<float> res = idsi_sum_.array_array_binop(nelements_idsi_, [&](float s, size_t n) {
        return n == 0 ? NAN : s / n;
    });
    Array<bool> all_set = all(nelements_idsi_ >= min_channel_increments, 0);
    for (size_t di = 0; di < inverse_depths_.length(); ++di) {
        for (size_t r = 0; r < res.shape(1); ++r) {
            for (size_t c = 0; c < res.shape(2); ++c) {
                if (!all_set(r, c)) {
                    res(di, r, c) = NAN;
                }
            }
        }
    }
    return std::make_unique<InverseDepthCostVolume>(res);
}
