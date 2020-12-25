#include "Project_Points.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Optimize/Levenberg_Marquardt.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <Mlib/Math/Rodrigues.hpp>
#include <Mlib/Math/Svd4.hpp>

using namespace Mlib;
using namespace Mlib::Cv;

Array<float> Mlib::Cv::k_external(const Array<float>& kep) {
    assert(all(kep.shape() == ArrayShape{6}));
    // auto ro = rodrigues<float>(Array<float>{k(0), k(1), k(2)});
    auto ro = tait_bryan_angles_2_matrix(Array<float>{kep(0), kep(1), kep(2)});
    return assemble_homogeneous_3x4(ro, Array<float>{kep(3), kep(4), kep(5)});
}

Array<float> Mlib::Cv::k_external_inverse(const Array<float>& ke) {
    Array<float> R = R3_from_Nx4(ke, 3);
    Array<float> t = t3_from_Nx4(ke, 3);
    Array<float> angles = matrix_2_tait_bryan_angles(R);
    return Array<float>{
        angles(0), angles(1), angles(2),
        t(0), t(1), t(2)};
}

Array<float> Mlib::Cv::k_internal(const Array<float>& kip) {
    assert(kip.length() == 4);
    return Array<float>{
        {kip(0), 0, kip(1)},
        {0, kip(2), kip(3)},
        {0, 0, 1}};
}

Array<float> Mlib::Cv::projected_points(
    const Array<float>& x,
    const Array<float>& ki,
    const Array<float>& ke,
    bool allow_points_at_infinity)
{
    assert(x.ndim() == 2);
    assert(x.shape(1) == 4);
    assert(all(ki.shape() == ArrayShape{3, 3}));
    assert(ke.ndim() == 3);
    assert(all(ke.shape().erased_first() == ArrayShape{3, 4}));
    // std::cerr << x.shape() << " | " << ki.shape() << " | " << ke.shape() << std::endl;
    Array<float> y(ArrayShape{ke.shape(0), x.shape(0), 3});
    for (size_t i = 0; i < ke.shape(0); ++i) {
        // std::cerr << ki.shape() << " | " << ke[i].shape() << std::endl;
        auto m = dot(ki, ke[i]);
        // auto kk = R3_from_Nx4(ke[i], 3);
        // std::cerr << (kk.T(), kk.T()) << std::endl;
        // std::cerr << (kk, kk) << std::endl;
        // std::cerr << ki << std::endl;
        // std::cerr << m.shape() << " - " << p.shape() << std::endl;
        // std::cerr << p[i].shape() << " . " << (m, P).T().shape() << std::endl;
        y[i] = outer(m, x).T();
    }
    // std::cerr << "y\n" << y << std::endl;
    for (size_t r = 0; r < y.shape(0); ++r) {
        for (size_t c = 0; c < y.shape(1); ++c) {
            const float t = y(r, c, 2);
            // also fails for NaN
            if (std::isnan(t)) {
                throw std::runtime_error("projected_points: t is NaN");
            }
            if (std::abs(t) < 1e-6) {
                if (allow_points_at_infinity) {
                    y(r, c, 2) = 0;
                } else {
                    throw std::runtime_error("projected_points: t too small: " + std::to_string(t));
                }
            } else {
                y(r, c, 0) /= t;
                y(r, c, 1) /= t;
                y(r, c, 2) /= t;
            }
        }
    }
    return y;
}

Array<float> Mlib::Cv::projected_points_1ke(
    const Array<float>& x,
    const Array<float>& ki,
    const Array<float>& ke,
    bool allow_points_at_infinity)
{
    assert(x.ndim() == 2);
    assert(x.shape(1) == 4);
    assert(all(ki.shape() == ArrayShape{3, 3}));
    assert(all(ke.shape() == ArrayShape{3, 4}));
    Array<float> res = projected_points(
        x,
        ki,
        ke.reshaped(ArrayShape{1}.concatenated(ke.shape())),
        allow_points_at_infinity);
    assert(all(res.shape() == ArrayShape{1, x.shape(0), 3}));
    return res[0];
}

Array<float> Mlib::Cv::projected_points_1p_1ke(
    const Array<float>& x,
    const Array<float>& ki,
    const Array<float>& ke,
    bool allow_points_at_infinity)
{
    assert(x.length() == 4);
    Array<float> res = projected_points_1ke(
        x.reshaped(ArrayShape{1}.concatenated(x.shape())),
        ki,
        ke,
        allow_points_at_infinity);
    assert(all(res.shape() == ArrayShape{1, 3}));
    return res[0];
}

/*
 * y = proj <- a <- K x
 * This function computes dy / dx.
 */
Array<float> Mlib::Cv::projected_points_jacobian_dx_1p_1ke(
    const Array<float>& x,
    const Array<float>& ki,
    const Array<float>& ke)
{
    assert(x.length() == 4);
    assert(all(ki.shape() == ArrayShape{3, 3}));
    assert(all(ke.shape() == ArrayShape{3, 4}));
    return homogeneous_jacobian_dx(dot(ki, ke), x);
}

/*
 * y = proj <- a <- K x
 * This function computes dy / dk,
 * by computing dy / da and da / dk.
 *
 * See: projected_points_jacobian_dke_1p_1ke_only_rotation
 */
Array<float> Mlib::Cv::projected_points_jacobian_dke_1p_1ke(
    const Array<float>& x,
    const Array<float>& ki,
    const Array<float>& kep)
{
    assert(x.length() == 4);
    assert(all(ki.shape() == ArrayShape{3, 3}));
    assert(kep.length() == 6);
    Array<float> ke = k_external(kep);
    Array<float> dy_da = homogeneous_jacobian_dx(ki, dot1d(ke, x));
    // Array<float> da_dkep = rodrigues_jacobian_dk(kep.row_range(0, 3), x.row_range(0, 3));

    // Numerical differentiation
    // Array<float> da_dkep = numerical_differentiation([&](
    //     const Array<float>& kk){ return (rodrigues(kk(0), kk(1), kk(2)), x.row_range(0, 3)); },
    //     kep.row_range(0, 3),
    //     float(1e-3));
    Array<float> da_dkep = tait_bryan_angles_dtheta(
        kep.row_range(0, 3),
        x.row_range(0, 3));

    Array<float> da_dk{
        {da_dkep(0, 0), da_dkep(0, 1), da_dkep(0, 2), 1, 0, 0},
        {da_dkep(1, 0), da_dkep(1, 1), da_dkep(1, 2), 0, 1, 0},
        {da_dkep(2, 0), da_dkep(2, 1), da_dkep(2, 2), 0, 0, 1}};
    return dot(dy_da, da_dk);
}

Array<float> Mlib::Cv::projected_points_jacobian_dki_1p_1ke(
    const Array<float>& x,
    const Array<float>& kip,
    const Array<float>& ke)
{
    assert(x.length() == 4);
    assert(kip.length() == 4);
    assert(all(ke.shape() == ArrayShape{3, 4}));
    Array<float> p = dot1d(ke, x);
    // ki = kip(0), 0      kip(1)
    //      0       kip(2) kip(3)
    //      0,      0      1
    // y = ki * p / x(2) = | kip(0) * p(0) + kip(1) * p(2) | / x(2)
    //                     | kip(2) * p(1) + kip(3) * p(2) |
    //                     | x(3)                          |
    Array<float> m_dy_dkip{
        {p(0), p(2), 0, 0},
        {0, 0, p(1), p(2)}};
    return m_dy_dkip / p(2);
}

/*
 * Only works with normalized coordinates
 */
Array<float> Mlib::Cv::reconstructed_point(
    const Array<float>& y_tracked,
    const Array<float>& ki,
    const Array<float>& ke,
    const Array<float>* weights,
    Array<float>* fs,
    bool method2,
    bool points_are_normalized,
    float *condition_number)
{
    assert(y_tracked.ndim() == 2);
    assert(y_tracked.shape(1) == 3);
    assert(all(ki.shape() == ArrayShape{3, 3}));
    assert(ke.ndim() == 3);
    assert(ke.shape(0) == y_tracked.shape(0));
    assert(all(ke.shape().erased_first() == ArrayShape{3, 4}));
    if (weights != nullptr) {
        assert(weights->length() == y_tracked.shape(0));
    }
    if (fs != nullptr) {
        assert(all(fs->shape() == ArrayShape{y_tracked.shape(0), 3}));
    }
    //f ----a---- f+a
    //  ----x-f--  | f+a-x
    //            \x
    //vv^T(x-f)=a
    //f+a-x=f-vv^Tf+(vv^T-I)x
    //(vv^T-I)x=(vv^T-I)f
    //f = e.g. camera center
    Array<float> M;
    if (method2) {
        M = Array<float>(ArrayShape{y_tracked.shape(0) * 3, y_tracked.shape(0) + 3});
        M = 0;
    } else {
        M = Array<float>(ArrayShape{y_tracked.shape(0) * 3, 3});
    }
    Array<float> B(ArrayShape{y_tracked.shape(0) * 3});
    for (size_t r = 0; r < y_tracked.shape(0); ++r) {
        Array<float> v;
        if (points_are_normalized) {
            Array<float> K = dot(ki, ke[r]);
            Array<float> K3x3 = R3_from_Nx4(K, 3);
            Array<float> yl2 = y_tracked[r];
            v = lstsq_chol_1d(K3x3.casted<double>(), yl2.casted<double>()).casted<float>(); //lstsq_chol_1d(K, y_tracked[r], float(1e-6));
        } else {
            Array<float> K3x3 = R3_from_Nx4(ke[r], 3);
            // Multiplying by "ki" first to scale the results into world-coordinates.
            v = lstsq_chol_1d(K3x3.casted<double>(), lstsq_chol_1d(ki.casted<double>(), y_tracked[r].casted<double>())).casted<float>();
        }
        v /= std::sqrt(sum(squared(v)));
        //std::cerr << "||v|| " << sum(squared(v)) << std::endl;
        //std::cerr << "yt " << y_tracked[r] << std::endl;
        //std::cerr << "Kv " << (K3x3, v) << std::endl;

        //proj * f = [0; 0; 0; 1] => f = proj \ [0; 0; 0; 1]
        Array<float> f = lstsq_chol_1d(
            homogenized_4x4(ke[r]),
            dirac_array<float>(ArrayShape{4}, ArrayShape{3})).row_range(0, 3);
        if (fs != nullptr) {
            (*fs)[r] = f;
        }
        //std::cerr << "hke\n" << homogenized_4x4(ke[r]) << std::endl;
        //std::cerr << "Ki " << Ki << std::endl;
        //std::cerr << "K " << K << std::endl;
        //std::cerr << "f " << f << std::endl;
        //std::cerr << "ke f " << (ke[r], Array<float>{f(0), f(1), f(2), 1}) << std::endl;

        if (method2) {
            for (size_t rr = 0; rr < 3; ++rr) {
                for (size_t c = 0; c < 3; ++c) {
                    M(rr + 3 * r, c) = (rr == c);
                }
                M(rr + 3 * r, 3 + r) = -v(rr);
            }
            B.row_range(3 * r, 3 * (r + 1)) = f;
        } else {
            Array<float> v2 = v.reshaped(ArrayShape{v.length(), 1});
            Array<float> m = outer(v2, v2) - identity_array<float>(3);
            Array<float> b = dot1d(m, f);
            if (weights != nullptr) {
                m *= (*weights)(r);
                b *= (*weights)(r);
            }
            M.row_range(3 * r, 3 * (r + 1)) = m;
            B.row_range(3 * r, 3 * (r + 1)) = b;
        }
    }
    //std::cerr << "M " << M << std::endl;
    //std::cerr << "B " << B << std::endl;
    if (condition_number != nullptr) {
        *condition_number = cond4_x(M.casted<double>());
    }
    if (method2) {
        return lstsq_chol_1d(M, B).row_range(0, 3);
    } else {
        return lstsq_chol_1d(
            M.casted<double>(),
            B.casted<double>()).casted<float>();
    }
}

Array<float> Mlib::Cv::reconstructed_point_reweighted(
    const Array<float>& y_tracked,
    const Array<float>& ki,
    const Array<float>& ke)
{
    Array<float> fs{ArrayShape{y_tracked.shape(0), 3}};
    Array<float> x = reconstructed_point(y_tracked, ki, ke, nullptr, &fs, false);
    for (size_t i = 0; i < 1; ++i) {
        Array<float> weights{ArrayShape{y_tracked.shape(0)}};
        for (size_t r = 0; r < weights.length(); ++r) {
            weights(r) =  1 / std::sqrt(sum(squared(x - fs[r])));
        }
        x = reconstructed_point(y_tracked, ki, ke, &weights, nullptr, false);
    }
    return x;
}
