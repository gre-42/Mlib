#pragma once
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Filters/Central_Differences.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Optimize/Levenberg_Marquardt.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <Mlib/Math/Rodrigues.hpp>
#include <Mlib/Sfm/Rigid_Motion/Rigid_Motion_Sampler.hpp>

namespace Mlib { namespace Sfm { namespace Rmfi {

template <class TData>
Array<TData> transform_coordinates(const Array<TData>& ki, const Array<TData>& ke, const Array<TData>& x, TData depth) {
    Array<TData> iki{inv(ki)};
    Array<TData> T{dot(ki, ke)};

    Array<TData> res3{dot(T, homogenized_4(dot(iki, homogenized_3(x)) * depth))};
    return Array<TData>{res3(0) / res3(2), res3(1) / res3(2)};
}

Array<float> projected_points_jacobian_dke_1p_1ke_lifting(
    const Array<float>& y,
    float depth,
    const Array<float>& ki,
    const Array<float>& kep)
{
    Array<float> x = lstsq_chol_1d(ki, homogenized_3(y)) * depth;
    return Cv::projected_points_jacobian_dke_1p_1ke(homogenized_4(x), ki, kep);
}

template <class TData>
Array<TData> d_pr_bilinear(
    const Array<TData>& im_r,
    const Array<TData>& im_l,
    const Array<TData>& im_r_depth,
    const Array<TData>& ki,
    const Array<TData>& ke)
{
    assert(im_r.ndim() == 2);
    assert(all(im_r.shape() == im_l.shape()));
    Array<TData> result{im_r.shape()};
    FixedArray<size_t, 2> shape{result.shape()};
    RigidMotionSampler rs{ki, ke, im_r_depth};
    #pragma omp parallel for
    for (size_t r = 0; r < result.shape(0); ++r) {
        for (size_t c = 0; c < result.shape(1); ++c) {
            BilinearInterpolator<TData> bi;
            if (!std::isnan(im_r_depth(r, c)) && rs.sample_destination(r, c, bi)) {
                result(r, c) = bi.interpolate_grayscale(im_l) - im_r(r, c);
            } else {
                result(r, c) = NAN;
            }
        }
    }
    return result;
}

template <class TData>
Array<TData> intensity_jacobian(
    const Array<TData>& im_r_di,
    const Array<TData>& im_l_di,
    const Array<TData>& im_r_depth,
    const Array<TData>& ki,
    const Array<TData>& kep)
{
    ArrayShape space_shape = im_r_di.shape().erased_first();
    Array<TData> result{space_shape.appended(6)};
    RigidMotionSampler hs{ki, Cv::k_external(kep), im_r_depth};
    #pragma omp parallel for
    for (size_t r = 0; r < im_r_di.shape(1); ++r) {
        for (size_t c = 0; c < im_r_di.shape(2); ++c) {
            if (std::isnan(im_r_depth(r, c))) {
                result[r][c] = NAN;
                continue;
            }
            ArrayShape id_r{r, c};
            Array<float> J = projected_points_jacobian_dke_1p_1ke_lifting(i2a(id_r), im_r_depth(r, c), ki, kep);
            Array<float> im_grad{im_r_di(id1, r, c), im_r_di(id0, r, c)};
            BilinearInterpolator<TData> bi;
            if (hs.sample_destination(r, c, bi)) {
                im_grad(0) = (im_grad(0) + bi.interpolate_multichannel(im_l_di, id1)) / 2;
                im_grad(1) = (im_grad(1) + bi.interpolate_multichannel(im_l_di, id0)) / 2;
            }
            result[r][c] = dot(im_grad, J);
        }
    }
    return result;
}

template <class TData>
Array<TData> intensity_jacobian_fast(
    const Array<TData>& im_r_di,
    const Array<TData>& im_l_di,
    const Array<TData>& im_r_depth,
    const Array<TData>& ki,
    const Array<TData>& kep)
{
    FixedArray<TData, 3, 3> iki{inv(ki)};
    FixedArray<TData, 3, 3> kif{ki};
    FixedArray<TData, 3, 4> ke{Cv::k_external(kep)};
    static const Array<TData> I = identity_array<TData>(3);
    // Changed order to be compatible with the rodrigues-implementation
    static const Array<TData> I0 = I[0];
    static const Array<TData> I1 = I[1];
    static const Array<TData> I2 = I[2];

    FixedArray<TData, 3, 3> R0{rodrigues(I0, kep(0))};
    FixedArray<TData, 3, 3> R1{rodrigues(I1, kep(1))};
    FixedArray<TData, 3, 3> R2{rodrigues(I2, kep(2))};

    FixedArray<TData, 3, 3> cross0{cross(I0)};
    FixedArray<TData, 3, 3> cross1{cross(I1)};
    FixedArray<TData, 3, 3> cross2{cross(I2)};

    FixedArray<TData, 3, 3> RR2 = dot(R2, cross2);
    FixedArray<TData, 3, 3> RR1 = dot(R2, dot(R1, cross1));
    FixedArray<TData, 3, 3> RR0 = dot(R2, dot(R1, dot(R0, cross0)));

    ArrayShape space_shape = im_r_di.shape().erased_first();
    Array<TData> result{space_shape.appended(6)};
    RigidMotionSampler hs{ki, Cv::k_external(kep), im_r_depth};
    #pragma omp parallel for
    for (size_t r = 0; r < im_r_di.shape(1); ++r) {
        for (size_t c = 0; c < im_r_di.shape(2); ++c) {
            if (std::isnan(im_r_depth(r, c))) {
                for (size_t i = 0; i < 6; ++i) {
                    result(r, c, i) = NAN;
                }
                continue;
            }
            FixedArray<size_t, 2> id_r{r, c};
            FixedArray<TData, 3> x = dot(iki, homogenized_3(i2a(id_r))) * im_r_depth(r, c);

            const auto m = kif.template row_range<0, 2>();
            const auto Mx = dot(kif, dot(ke, homogenized_4(x)));
            const auto mx = Mx.template row_range<0, 2>();
            const TData bx = Mx(2);

            const auto mx_2d = mx.template reshaped<2, 1>();
            const auto b_2d = kif.template row_range<2, 3>();

            FixedArray<TData, 2, 3> dy_da = ((m * bx) - dot(mx_2d, b_2d)) / squared(bx);

            FixedArray<TData, 3> r0 = dot(R0, x);
            FixedArray<TData, 3> r1 = dot(R1, r0);

            FixedArray<TData, 3, 3> da_dkep_T;
            da_dkep_T[0] = dot(RR0, x);
            da_dkep_T[1] = dot(RR1, r0);
            da_dkep_T[2] = dot(RR2, r1);

            FixedArray<float, 3, 6> da_dk{
                da_dkep_T(0, 0), da_dkep_T(1, 0), da_dkep_T(2, 0), 1, 0, 0,
                da_dkep_T(0, 1), da_dkep_T(1, 1), da_dkep_T(2, 1), 0, 1, 0,
                da_dkep_T(0, 2), da_dkep_T(1, 2), da_dkep_T(2, 2), 0, 0, 1};

            FixedArray<TData, 2, 6> J = dot(dy_da, da_dk);

            FixedArray<TData, 2> im_grad{im_r_di(id1, r, c), im_r_di(id0, r, c)};
            BilinearInterpolator<TData> bi;
            if (hs.sample_destination(r, c, bi)) {
                im_grad(0) = (im_grad(0) + bi.interpolate_multichannel(im_l_di, id1)) / 2;
                im_grad(1) = (im_grad(1) + bi.interpolate_multichannel(im_l_di, id0)) / 2;
            }
            FixedArray<TData, 6> intensity = dot(im_grad, J);
            for (size_t i = 0; i < 6; ++i) {
                result(r, c, i) = intensity(i);
            }
        }
    }
    return result;
}

template <class TData>
Array<TData> rigid_motion_from_images(
    const Array<TData>& im_r,
    const Array<TData>& im_l,
    const Array<TData>& im_r_depth,
    const Array<TData>& intrinsic_matrix,
    bool differentiate_numerically = false,
    const Array<TData>* x0 = nullptr,
    Array<TData>* xe = nullptr)
{
    assert(im_r.ndim() == 2);
    assert(all(im_r.shape() == im_l.shape()));
    if (x0 != nullptr) {
        assert(x0->length() == 6);
    }
    const TData NAN_VALUE = 0;
    auto f = [&](const Array<TData>& x){
        return substitute_nans(d_pr_bilinear(im_r, im_l, im_r_depth, intrinsic_matrix, Cv::k_external(x)).flattened(), NAN_VALUE);
    };
    Array<float> im_r_di = central_gradient_filter(im_r);
    Array<float> im_l_di = central_gradient_filter(im_l);
    Array<TData> xx = levenberg_marquardt(
        x0 == nullptr ? zeros<TData>(ArrayShape{6}) : *x0,
        zeros<TData>(ArrayShape{im_r.nelements()}),
        f,
        [&](const Array<TData>& x){
            return differentiate_numerically
                ? numerical_differentiation(f, x)
                : substitute_nans(intensity_jacobian_fast(im_r_di, im_l_di, im_r_depth, intrinsic_matrix, x).rows_as_1D(), NAN_VALUE);
        },
        TData(1e-2),   // alpha,
        TData(1e-2),   // beta,
        TData(1e-2),   // alpha2,
        TData(1e-2),   // beta2,
        TData(0),      // min_redux
        100,           // niterations
        5,             // nburnin
        30);           // nmisses
    if (xe != nullptr) {
        *xe = xx;
    }
    return Cv::k_external(xx);
}

}}}
