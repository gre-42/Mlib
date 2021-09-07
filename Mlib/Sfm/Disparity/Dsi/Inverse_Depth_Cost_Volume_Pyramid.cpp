#include "Inverse_Depth_Cost_Volume_Pyramid.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Images/Resample/Down_Sample_Average.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Sfm/Homography/Homography_From_Transform.hpp>
#include <Mlib/Sfm/Homography/Homography_Sampler.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>
#include <Mlib/Stats/Min_Max.hpp>

using namespace Mlib;
using namespace Mlib::Sfm;

InverseDepthCostVolumePyramid::InverseDepthCostVolumePyramid(const Array<float>& arr)
: arr_(arr)
{
    assert(arr.ndim() == 4);
}

Array<float> InverseDepthCostVolumePyramid::dsi() const {
    return sum(abs(arr_), 0);
}

std::unique_ptr<CostVolume> InverseDepthCostVolumePyramid::down_sampled() const {
    Array<float> ar2 = arr_.reshaped(ArrayShape{ arr_.shape(0) * arr_.shape(1) }.concatenated(arr_.shape().erased_first(2)));
    Array<float> ds = multichannel_down_sample_average(ar2);
    return std::make_unique<InverseDepthCostVolumePyramid>(ds.reshaped(ArrayShape{ arr_.shape(0), arr_.shape(1) }.concatenated(ds.shape().erased_first())));
}

InverseDepthCostVolumePyramidAccumulator::InverseDepthCostVolumePyramidAccumulator(
    const ArrayShape& image_shape,
    const Array<float>& inverse_depths)
: image_shape_(image_shape),
  inverse_depths_(inverse_depths),
  idsi_sum_(zeros<float>(ArrayShape{image_shape(0), inverse_depths.length()}.concatenated(image_shape.erased_first()))),
  nelements_idsi_(zeros<size_t>(idsi_sum_.shape())),
  nchannel_increments_(0)
{
    assert(image_shape.ndim() == 3);
    assert(inverse_depths.ndim() == 1);
}

void InverseDepthCostVolumePyramidAccumulator::increment(
    const TransformationMatrix<float, 2>& intrinsic_matrix,
    const TransformationMatrix<float, 3>& ke0,
    const TransformationMatrix<float, 3>& ke1,
    const Array<float>& im0_rgb,
    const Array<float>& im1_rgb,
    const float epipole_radius)
{
    assert(im0_rgb.ndim() == 3);
    assert(im0_rgb.shape(0) == idsi_sum_.shape(0));
    assert(all(im0_rgb.shape() == im1_rgb.shape()));
    assert(all(im0_rgb.shape() == image_shape_));

    nchannel_increments_ += im0_rgb.shape(0);

    TransformationMatrix<float, 3> ke = projection_in_reference(ke0, ke1);
    FixedArray<float, 2> e;
    if (epipole_radius != 0) {
        if (max(abs(ke.affine() - fixed_identity_array<float, 4>())) > 1e-3) {
            FixedArray<float, 2> e2 = find_epipole(intrinsic_matrix, ke);
            if (!all(Mlib::isnan(e2))) {
                e = a2fi(e2);
            }
        }
    }

    ArrayShape space_shape{ image_shape_(1), image_shape_(2) };
    for (size_t di = 0; di < inverse_depths_.length(); ++di) {
        FixedArray<float, 3, 3> homog_e = rotation_and_translation_to_homography(
            ke,
            -FixedArray<float, 3>{0.f, 0.f, 1.f},
            1.f / inverse_depths_(di));
        FixedArray<float, 3, 3> homog_i = pixel_homography(intrinsic_matrix, homog_e);
        HomographySampler<float> hs{homog_i};
        #pragma omp parallel for
        for (int ri = 0; ri < (int)im0_rgb.shape(1); ++ri) {
            size_t r = (size_t)ri;
            for (size_t c = 0; c < im0_rgb.shape(2); ++c) {
                if ((epipole_radius != 0) && !any(Mlib::isnan(e)) && (std::abs(r - e(0)) < epipole_radius) && (std::abs(c - e(1)) < epipole_radius)) {
                    continue;
                }
                BilinearInterpolator<float> bi;
                if (hs.sample_destination(r, c, space_shape, bi)) {
                    for (size_t h = 0; h < im0_rgb.shape(0); ++h) {
                        float v = bi(im1_rgb, h);
                        if (!std::isnan(v)) {
                            idsi_sum_(h, di, r, c) += (im0_rgb(h, r, c) - v);
                            ++nelements_idsi_(h, di, r, c);
                        }
                    }
                }
            }
        }
    }
}

std::unique_ptr<CostVolume> InverseDepthCostVolumePyramidAccumulator::get(size_t min_channel_increments) const {
    if (nchannel_increments_ < min_channel_increments) {
        throw std::runtime_error("nincrements is smaller than min_channel_increments");
    }
    if (min_channel_increments % nelements_idsi_.shape(0) != 0) {
        throw std::runtime_error("min_channel_increments is not a multiple of nchannels");
    }
    Array<float> res = idsi_sum_.array_array_binop(nelements_idsi_, [&](float s, size_t n) {
        return n == 0 ? NAN : s / n;
    });
    ArrayShape eshape = ArrayShape{nelements_idsi_.shape(0) * nelements_idsi_.shape(1)}.concatenated(nelements_idsi_.shape().erased_first(2));
    Array<bool> all_set = all(nelements_idsi_.reshaped(eshape) >= (min_channel_increments / nelements_idsi_.shape(0)), 0);
    for (size_t di = 0; di < inverse_depths_.length(); ++di) {
        for (size_t r = 0; r < res.shape(1); ++r) {
            for (size_t c = 0; c < res.shape(2); ++c) {
                if (!all_set(r, c)) {
                    for (size_t h = 0; h < res.shape(0); ++h) {
                        res(h, di, r, c) = NAN;
                    }
                }
            }
        }
    }
    return std::make_unique<InverseDepthCostVolumePyramid>(res);
}
