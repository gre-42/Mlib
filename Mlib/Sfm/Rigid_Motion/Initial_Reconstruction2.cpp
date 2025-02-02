#include "Initial_Reconstruction2.hpp"
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Optimize/Levenberg_Marquardt.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Stats/Ransac.hpp>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

static Array<FixedArray<float, 3>> unpack_x(const Array<float>& k, size_t nx);
static Array<TransformationMatrix<float, float, 3>> unpack_k_external(const Array<float>& k, size_t nintrinsic, size_t nx);

static TransformationMatrix<float, float, 2> unpack_k_internal(const Array<float>& k) {
    assert(k.ndim() == 1);
    return k_internal(FixedArray<float, 4>{k.row_range(0, 4)});
}

static Array<FixedArray<float, 2, 4>> unpack_k_internal_dk(
    const Array<FixedArray<float, 3>>& x,
    const Array<TransformationMatrix<float, float, 3>>& ke)
{
    // t, i, y, k
    Array<FixedArray<float, 2, 4>> result{ArrayShape{ke.shape(0), x.length()}};
    for (size_t t = 0; t < ke.shape(0); ++t) {
        for (size_t i = 0; i < x.length(); ++i) {
            result(t, i) = projected_points_jacobian_dki_1p_1ke(x(i), ke(t));
        }
    }
    return result;
}

static Array<TransformationMatrix<float, float, 3>> unpack_k_external(const Array<float>& k, size_t nintrinsic, size_t nx) {
    assert(k.ndim() == 1);
    assert((k.shape(0) - nintrinsic - nx) % 6 == 0);
    Array<TransformationMatrix<float, float, 3>> result{ArrayShape{(k.shape(0) - nintrinsic - nx) / 6}};
    for (size_t i = nintrinsic; i < k.shape(0) - nx; i += 6) {
        result[(i - nintrinsic) / 6] = k_external(FixedArray<float, 6>{k.row_range(i, i + 6)});
    }
    return result;
}

static Array<FixedArray<float, 2, 6>> unpack_k_external_dk(
    const TransformationMatrix<float, float, 2>& ki,
    const Array<float>& k,
    size_t nintrinsic,
    size_t nx,
    const Array<FixedArray<float, 3>>& x)
{
    assert(k.ndim() == 1);
    assert((k.shape(0) - nintrinsic - nx) % 6 == 0);
    // t, i, y, k
    Array<FixedArray<float, 2, 6>> result{ArrayShape{(k.shape(0) - nintrinsic - nx) / 6, x.length()}};
    for (size_t t = nintrinsic; t < k.shape(0) - nx; t += 6) {
        for (size_t i = 0; i < x.length(); ++i) {
            result((t - nintrinsic) / 6, i) = projected_points_jacobian_dke_1p_1ke(
                x(i),
                ki,
                FixedArray<float, 6>{ k.row_range(t, t + 6) });
        }
    }
    return result;
}

static Array<FixedArray<float, 3>> unpack_x(const Array<float>& k, size_t nx) {
    assert(nx > 0);
    assert(nx % 3 == 0);
    Array<FixedArray<float, 3>> x{ ArrayShape{nx / 3} };
    Array<float> k3 = k.row_range(k.shape(0) - nx, k.shape(0));
    k3.do_reshape(ArrayShape{nx / 3, 3});
    for (size_t i = 0; i < x.length(); ++i) {
        for (size_t d = 0; d < 3; ++d) {
            x(i)(d) = k3(i, d);
        }
    }
    return x;
}

static void pack_x(const Array<FixedArray<float, 3>>& x, Array<float>& k, size_t nx) {
    assert(nx > 0);
    assert(nx % 3 == 0);
    assert(x.ndim() == 1);
    assert(k.ndim() == 1);
    Array<float> k3 = k.row_range(k.shape(0) - nx, k.shape(0));
    k3.do_reshape(ArrayShape{ nx / 3, 3 });
    for (size_t i = 0; i < x.length(); ++i) {
        for (size_t d = 0; d < 3; ++d) {
            k3(i, d) = x(i)(d);
        }
    }
}

static Array<float> pack_y(const Array<FixedArray<float, 2>>& y) {
    return Array<float>{y}.flattened();
}

static Array<FixedArray<float, 2, 3>> unpack_x_dx(
    const TransformationMatrix<float, float, 2>& ki,
    const Array<FixedArray<float, 3>>& x,
    const Array<TransformationMatrix<float, float, 3>>& ke)
{
    // t, i, y, x
    Array<FixedArray<float, 2, 3>> result{ArrayShape{ke.length(), x.length()}};
    for (size_t t = 0; t < ke.length(); ++t) {
        for (size_t i = 0; i < x.length(); ++i) {
            result(t, i) = projected_points_jacobian_dx_1p_1ke(x(i), ki, ke(t));
        }
    }
    return result;
}

/*
 * Only works with normalized coordinates
 */
Array<FixedArray<float, 3>> Mlib::Sfm::initial_reconstruction(
    const TransformationMatrix<float, float, 3>& ke,
    const TransformationMatrix<float, float, 2>& ki,
    const Array<FixedArray<float, 2>>& y0,
    const Array<FixedArray<float, 2>>& y1,
    bool points_are_normalized,
    Array<float>* condition_number)
{
    assert(y0.ndim() == 1);
    assert(y0.length() == y1.length());
    if (condition_number != nullptr) {
        if (!condition_number->initialized()) {
            condition_number->do_resize(ArrayShape{y0.length()});
        }
        assert(condition_number->length() == y0.length());
    }

    Array<TransformationMatrix<float, float, 3>> kes{
        TransformationMatrix<float, float, 3>::identity(),
        ke };
    Array<FixedArray<float, 3>> x(ArrayShape{y0.length()});
    Array<FixedArray<float, 2>> y_tracked(ArrayShape{2});
    for (size_t i = 0; i < y0.length(); ++i) {
        y_tracked(0) = y0(i);
        y_tracked(1) = y1(i);
        x(i) = reconstructed_point_(y_tracked, ki, kes,
            nullptr, // weights
            false,   // method2
            points_are_normalized,
            condition_number == nullptr ? nullptr : &(*condition_number)(i));
    }
    return x;
}

/*
 * Reconstruct only the 3rd component (z) of x.
 * Did not get this working.
 * R.T seems to work better than R but it is then still not exact.
 * Using "initial_reconstruction" instead.
 */
Array<float> Mlib::Sfm::initial_reconstruction_x3(
    const TransformationMatrix<float, float, 3>& tm,
    const TransformationMatrix<float, float, 2>& ki,
    const Array<FixedArray<float, 2>>& y0,
    const Array<FixedArray<float, 2>>& y1,
    bool verbose)
{
    assert(all(y0.shape() == y1.shape()));
    assert(y0.ndim() == 1);

    //lerr() << "t " << t;
    //lerr() << "R\n" << R;
    //lerr() << y1;
    //lerr() << y1.shape();

    //x3: the third component of x, a.k.a. z
    Array<float> x3{ ArrayShape{ y0.length(), 2 } };
    if (verbose) {
        lerr() << "R\n" << tm.R;
        lerr() << "t " << tm.t;
        lerr() << "y0 " << lstsq_chol_1d(ki.affine(), homogenized_3(y0(0))).value();
        lerr() << "y1 " << lstsq_chol_1d(ki.affine(), homogenized_3(y1(0))).value();
    }
    for (size_t i = 0; i < y0.length(); ++i) {
        const FixedArray<float, 3> yy0 = lstsq_chol_1d(ki.affine(), homogenized_3(y0(i))).value();
        const FixedArray<float, 3> yy1 = lstsq_chol_1d(ki.affine(), homogenized_3(y1(i))).value();
        FixedArray<float, 3> c0 = tm.R[0] - yy1(0) * tm.R[2];
        FixedArray<float, 3> c1 = tm.R[1] - yy1(1) * tm.R[2];
        if (verbose) {
            lerr() << "yy0 " << yy0;
            lerr() << "yy1 " << yy1;
            lerr() << "c0 " << c0;
            lerr() << "c1 " << c1;
            lerr() << "(c0, t) " << dot0d(c0, tm.t);
            lerr() << "(c0, yy0) " << dot0d(c0, yy0);
        }
        x3(i, 0) = dot0d(c0, tm.t) / dot0d(c0, yy0);
        x3(i, 1) = dot0d(c1, tm.t) / dot0d(c1, yy0);
    };
    if (verbose) {
        lerr();
        lerr() << "----------------";
        //lerr() << "x3\n" << x3;
        //lerr() << x3.shape();
        lerr() << "R\n" << tm.R;
        lerr() << "t " << tm.t;
        lerr() << "ok (>0): " << all(x3 > 0.f);
        //lerr() << "ok: " << min(x3);
        //The check should be about "> 0", but this was not enough.
        lerr() << "min(x3) " << min(x3);
    }
    return x3;
}

void Mlib::Sfm::find_projection_matrices(
    const Array<FixedArray<float, 3>>& x,
    const Array<FixedArray<float, 2>>& y,
    const TransformationMatrix<float, float, 2>* ki_precomputed,
    const Array<float>* kep_initial,
    TransformationMatrix<float, float, 2>* ki_out,
    Array<TransformationMatrix<float, float, 3>>* ke_out,
    Array<float>* kep_out,
    Array<FixedArray<float, 3>>* x_out,
    float alpha,
    float beta,
    float alpha2,
    float beta2,
    float min_redux,
    size_t niterations,
    size_t nburnin,
    size_t nmisses,
    bool print_residual,
    bool nothrow,
    Array<float>* final_residual,
    const float* max_residual,
    bool differentiate_numerically)
{
    assert(x.ndim() == 1);
    assert(y.ndim() == 2);
    assert(x.length() == y.shape(1));
    size_t nintrinsic = ki_precomputed != nullptr ? 0 : 4;
    size_t nx = x_out != nullptr ? x.length() * 3 : 0;
    Array<float> k0 = zeros<float>(ArrayShape{nintrinsic + y.shape(0) * 6 + nx});
    if (ki_precomputed == nullptr) {
        k0(0) = 1;
        k0(1) = 0;
        k0(2) = 1;
        k0(3) = 0;
    }
    if (kep_initial != nullptr) {
        assert(kep_initial->length() == y.shape(0) * 6);
        k0.row_range(
            nintrinsic,
            nintrinsic + y.shape(0) * 6) = *kep_initial;
    }
    if (x_out != nullptr) {
        pack_x(x, k0, nx);
    }
    // NormalizedProjection(np.pn).print_min_max();
    auto f = [&](const Array<float>& k) -> Array<float> {
        auto ki = (ki_precomputed != nullptr)
            ? *ki_precomputed
            : unpack_k_internal(k);
        auto ke = unpack_k_external(k, nintrinsic, nx);
        auto xx = (x_out == nullptr)
            ? x
            : unpack_x(k, nx);
        auto s = projected_points(xx, ki, ke);
        return pack_y(s.flattened());
    };
    Array<float> k_opt = levenberg_marquardt(
        k0,
        pack_y(y),
        f,
        [&](const Array<float>& k) -> Array<float> {
            if (differentiate_numerically) {
                return numerical_differentiation(f, k, float(1e-4));
            } else {
                TransformationMatrix<float, float, 2> ki = ki_precomputed != nullptr
                    ? *ki_precomputed
                    : unpack_k_internal(k);
                Array<TransformationMatrix<float, float, 3>> ke = unpack_k_external(k, nintrinsic, nx);
                Array<FixedArray<float, 3>> xx = (x_out == nullptr)
                    ? x
                    : unpack_x(k, nx);

                Array<float> J = zeros<float>(ArrayShape{y.shape(0), y.shape(1), 2, k.length()});
                Array<FixedArray<float, 2, 4>> dy_dki;
                if (ki_precomputed == nullptr) {
                    dy_dki = unpack_k_internal_dk(xx, ke);
                }
                Array<FixedArray<float, 2, 6>> dy_dke = unpack_k_external_dk(
                    ki,
                    k,
                    nintrinsic,
                    nx,
                    xx);
                Array<FixedArray<float, 2, 3>> dy_dx;
                if (x_out != nullptr) {
                    dy_dx = unpack_x_dx(ki, xx, ke);
                }
                for (size_t t = 0; t < y.shape(0); ++t) {
                    for (size_t i = 0; i < y.shape(1); ++i) {
                        for (size_t d = 0; d < 2; ++d) {
                            if (ki_precomputed == nullptr) {
                                for (size_t j = 0; j < 4; ++j) {
                                    J(t, i, d, j) = dy_dki(t, i)(d, j);
                                }
                            }
                            for (size_t j = 0; j < 6; ++j) {
                                J(t, i, d, nintrinsic + t * 6 + j) = dy_dke(t, i)(d, j);
                            }
                            if (x_out != nullptr) {
                                for (size_t j = 0; j < 3; ++j) {
                                    J(t, i, d, nintrinsic + y.shape(0) * 6 + i * 3 + j) = dy_dx(t, i)(d, j);
                                }
                            }
                        }
                    }
                }
                // auto an = J.rows_as_1D();
                // auto nu = numerical_differentiation(f, k, float(1e-4));
                // an.save_txt_2d("an.m");
                // nu.save_txt_2d("nu.m");
                // assert(false);
                // return nu;
                return J.rows_as_1D();
            }
        },
        alpha,
        beta,
        alpha2,
        beta2,
        min_redux,
        niterations,
        nburnin,
        nmisses,
        print_residual,
        nothrow,
        final_residual,
        max_residual);
    if (ki_out != nullptr) {
        *ki_out = ki_precomputed != nullptr ? *ki_precomputed : unpack_k_internal(k_opt);
    }
    if (ke_out != nullptr) {
        *ke_out = unpack_k_external(k_opt, nintrinsic, nx);
    }
    if (kep_out != nullptr) {
        *kep_out = k_opt.row_range(nintrinsic, nintrinsic + y.shape(0) * 6);
    }
    if (x_out != nullptr) {
        *x_out = unpack_x(k_opt, nx);
    }
    if (final_residual != nullptr) {
        assert(final_residual->length() == y.nelements());
        final_residual->do_reshape(y.shape());
    }
}

void Mlib::Sfm::find_projection_matrices_ransac(
    const RansacOptions<float>& ro,
    const Array<FixedArray<float, 3>>& x,
    const Array<FixedArray<float, 2>>& y,
    const TransformationMatrix<float, float, 2>* ki_precomputed,
    TransformationMatrix<float, float, 2>* ki_out,
    Array<TransformationMatrix<float, float, 3>>* ke_out,
    Array<float>* kep_out,
    Array<FixedArray<float, 3>>* x_out,
    float alpha,
    float beta,
    float alpha2,
    float beta2,
    float min_redux,
    size_t niterations,
    size_t nburnin,
    size_t nmisses,
    bool print_residual,
    bool nothrow,
    Array<float>* final_residual)
{
    auto gen_y1 = [&](const Array<size_t>& indices) {
        assert(y.ndim() == 2);
        Array<FixedArray<float, 2>> y1{ArrayShape{y.shape(0), indices.length()}};
        for (size_t i = 0; i < y.shape(0); ++i) {
            y1[i] = y[i][indices];
        }
        return y1;
    };

    const Array<size_t> best_indices = ransac(
        y.shape(1), // nelems_large
        ro,
        [&](const Array<size_t>& indices) {
            TransformationMatrix<float, float, 2> ki_out1 = uninitialized;
            Array<TransformationMatrix<float, float, 3>> ke_out1;
            find_projection_matrices(
                x[indices],
                gen_y1(indices),
                ki_precomputed,
                nullptr,           // kep_initial
                &ki_out1,
                &ke_out1,
                nullptr,           // kep_out
                nullptr,           // x_out
                alpha,
                beta,
                alpha2,
                beta2,
                min_redux,
                niterations,
                nburnin,
                nmisses,
                print_residual,
                nothrow);
            // y = (t, i, d)
            // Note that this is an input to the RANSAC function, not for the LM algorithm.
            Array<FixedArray<float, 2>> residual = projected_points(x, ki_out1, ke_out1) - y;
            Array<float> residual2 = zeros<float>(ArrayShape{y.shape(1)});
            for (size_t t = 0; t < residual.shape(0); ++t) {
                for (size_t i = 0; i < residual.shape(1); ++i) {
                    for (size_t d = 0; d < 2; ++d) {
                        residual2(i) += squared(residual(t, i)(d));
                    }
                }
            }
            return residual2;
        }
    );
    if (!best_indices.initialized()) {
        throw std::runtime_error("RANSAC could not find a good projection");
    }
    find_projection_matrices(
        x[best_indices],
        gen_y1(best_indices),
        ki_precomputed,
        nullptr,               //kep_initial
        ki_out,
        ke_out,
        kep_out,
        x_out,
        alpha,
        beta,
        alpha2,
        beta2,
        min_redux,
        niterations,
        nburnin,
        nmisses,
        print_residual,
        nothrow,
        final_residual);
}

void Mlib::Sfm::find_projection_matrices_twopass(
    const Array<FixedArray<float, 3>>& x,
    const Array<FixedArray<float, 2>>& y,
    const TransformationMatrix<float, float, 2>* ki_precomputed,
    TransformationMatrix<float, float, 2>* ki_out,
    Array<TransformationMatrix<float, float, 3>>* ke_out,
    Array<float>* kep_out,
    Array<FixedArray<float, 3>>* x_out,
    float alpha,
    float beta,
    float alpha2,
    float beta2,
    float min_redux,
    size_t niterations,
    size_t nburnin,
    size_t nmisses,
    bool print_residual,
    bool nothrow,
    Array<float>* final_residual,
    const float* max_residual)
{
    Array<float> kep_out1;
    find_projection_matrices(
        x,                 // x
        y,                 // y
        ki_precomputed,    // ki_precomputed
        nullptr,           // kep_initial
        nullptr,           // ki_out
        nullptr,           // ke_out
        &kep_out1,         // kep_out
        nullptr,           // x_out
        alpha,             // alpha
        beta,              // beta
        alpha2,            // alpha2
        beta2,             // beta2
        min_redux,         // min_redux
        niterations,       // niterations
        nburnin,           // nburnin
        nmisses,           // nmisses
        print_residual,    // print_residual
        nothrow,           // nothrow
        nullptr,           // final_residual
        max_residual);     // max_residual

    find_projection_matrices(
        x,                 // x
        y,                 // y
        ki_precomputed,    // ki_precomputed
        &kep_out1,         // kep_initial
        ki_out,            // ki_out
        ke_out,            // ke_out
        kep_out,           // kep_out
        x_out,             // x_out
        alpha,             // alpha
        beta,              // beta
        alpha2,            // alpha2
        beta2,             // beta2
        min_redux,         // min_redux
        niterations,       // niterations
        nburnin,           // nburnin
        nmisses,           // nmisses
        print_residual,    // print_residual
        nothrow,           // nothrow
        final_residual,    // final_residual
        max_residual);     // max_residual
}

FixedArray<float, 2> Mlib::Sfm::find_epipole(
    const TransformationMatrix<float, float, 2>& ki,
    const TransformationMatrix<float, float, 3>& ke)
{
    return projected_points_1p_1ke(
        ke.inverted().t,
        ki,
        TransformationMatrix<float, float, 3>::identity(),
        PointAtInfinityBehavior::IS_NAN);  // allow_points_at_fininity
}
