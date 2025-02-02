#include <Mlib/Math/Approximate_Rank.hpp>
#include <Mlib/Math/Eigen_Jacobi.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Test.hpp>
#include <Mlib/Math/Gaussian_Elimination.hpp>
#include <Mlib/Math/Huber_Norm.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Math/Interpolate.hpp>
#include <Mlib/Math/Inv.hpp>
#include <Mlib/Math/Least_Common_Multiple.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Non_Zero_Ids.hpp>
#include <Mlib/Math/Optimize/Cg.hpp>
#include <Mlib/Math/Power_Iteration/Cond.hpp>
#include <Mlib/Math/Power_Iteration/Inverse_Iteration.hpp>
#include <Mlib/Math/Power_Iteration/Pinv.hpp>
#include <Mlib/Math/Power_Iteration/Qdq.hpp>
#include <Mlib/Math/Power_Iteration/Svd.hpp>
#include <Mlib/Math/Set_Difference.hpp>
#include <Mlib/Math/Simd.hpp>
#include <Mlib/Math/Sort_Svd.hpp>
#include <Mlib/Math/Svd4.hpp>
#include <Mlib/Math/Svd_Jacobi.hpp>
#include <Mlib/Math/Transformation/Quaternion_Series.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Stats/Random_Arrays.hpp>
#include <Mlib/Time/Time_Guard.hpp>

using namespace Mlib;

void test_blocking_transposed() {
    Array<float> a{ ArrayShape{ 640, 480 } };
    TimeGuard::initialize(10 * 1000, MaxLogLengthExceededBehavior::THROW_EXCEPTION);
    for (size_t block_size = 1; block_size < 100; block_size += 5) {
        TimeGuard tg{ "asdf", std::to_string(block_size) };
        for (size_t i = 0; i < 10; ++i) {
            assert_isequal(a(60, 70), a.T(block_size).T(block_size)(60, 70));
        }
    }
    TimeGuard::print_groups(lraw().ref());
}

void test_svd() {
    Array<float> a = random_array<float>(ArrayShape{3, 2});
    Array<float> uT;
    Array<float> s;
    Array<float> vT;
    svd(a, uT, s, vT, 0);  // 0 = seed
    /*lraw() << std::setf(std::ios_base::scientific);
    lerr() << "a";
    lerr() << a;
    lerr() << "u";
    lerr() << uT.T();
    lerr() << "s";
    lerr() << s;
    lerr() << "v";
    lerr() << v;*/
    assert_isclose<float>(a(2, 1), 407);
    assert_allclose<float>(uT.T(), Array<float>{{(float)3.225679e-02, (float)1.277676e-01},
                                                {(float)6.286538e-01, (float)7.684482e-01},
                                                {(float)7.770160e-01, (float)-6.270269e-01}});
    assert_allclose<float>(s, Array<float>{(float)9.966705e+02, (float)2.68333649e+02});
    assert_allclose<float>(vT, Array<float>{{(float)7.294542e-01, (float)6.840296e-01},
                                            {(float)-6.840298e-01, (float)7.294541e-01}});

    Array<float> ui;
    float si;
    inverse_iteration_symm(dot(a.T(), a), ui, si);
    assert_allclose<float>(ui, Array<float>{(float)-6.840296e-01, (float)7.294543e-01});
    assert_isclose<float>(std::sqrt(si), 268.333618f, (float)1e-4);
}

void test_svd_j() {
    Array<double> a = random_array3<double>(ArrayShape{ 4, 4 }, 1);
    {
        Array<double> u;
        Array<double> s;
        Array<double> vT;
        svd4(a, u, s, vT);
        assert_allclose(reconstruct_svd(u, s, vT), a);
        lerr() << u;
        lerr() << s;
        lerr() << vT;
    }
    {
        Array<double> uT;
        Array<double> s;
        Array<double> vT;
        svd_j(a, uT, s, vT);
        lerr() << uT;
        lerr() << s;
        lerr() << vT;
        assert_allclose(reconstruct_svd(uT.T(), s, vT), a);
    }
}

void test_qdq() {
    Array<float> a = random_array<float>(ArrayShape{3, 3}, 1);
    Array<float> q;
    Array<float> s;
    a = -outer(a, a);
    qdq(a, q, s, 0);  // 0 = seed
    /*lraw() << std::setf(std::ios_base::scientific);
    lerr() << "a";
    lerr() << a;
    lerr() << "q";
    lerr() << q;
    lerr() << "s";
    lerr() << s;*/
    assert_isclose<float>(q(1, 0) / sign(q(0, 0)), (float)-0.149676);
    assert_isclose<float>(s(0), 447580.468750f);
    assert_isclose<float>(s(1), 64099.5f);
}

void test_det_2x2() {
    Array<float> a = random_array2<float>(ArrayShape{2, 2}, 1);
    Array<float> q;
    Array<float> s;
    a = outer(a, a) + identity_array<float>(2) * 0.01f;
    qdq(a, q, s);
    assert_isclose(det2x2(a), s(0) * s(1));
}

void test_det_3x3() {
    Array<float> a({
        {6, 1, 1},
        {4, -2, 5},
        {2, 8, 7}});
    assert_isclose<float>(det3x3(a), -306.f);
}

void test_identity() {
    Array<float> a;
    a.resize[5](4);
    identity_array(a);
    assert(a(0, 0) == 1);
    assert(a(1, 1) == 1);
    assert(a(1, 0) == 0);
}

void test_lstsq() {
    Array<float> a = random_array<float>(ArrayShape{5, 3}, 1);
    Array<float> b = random_array<float>(ArrayShape{5, 2}, 2);
    assert_allclose(dot(pinv(a), a), identity_array<float>(3), float(1e-4));
    assert_allclose(pinv(a), pinv(a.T()).T());
    assert_allclose(lstsq(a, a), identity_array<float>(3), float(1e-4));
    assert_allclose(lstsq(a, 1.4f * a), 1.4f * identity_array<float>(3), float(1e-4));
}

void test_lstsq_complex() {
    Array<std::complex<float>> ar;
    Array<std::complex<float>> ai;
    ar.resize[5](3);
    ai.resize[5](3);
    randomize_array(ar, 1);
    randomize_array(ai, 2);
    Array<std::complex<float>> a = ar + std::complex<float>(0, 1) * ai;
    assert_allclose(dot(pinv(a), a), identity_array<std::complex<float>>(3), float(1e-3));
    assert_allclose(lstsq(a, a), identity_array<std::complex<float>>(3), float(1e-3));
}

void test_lstsq_chol() {
    Array<float> a = random_array<float>(ArrayShape{5, 3}, 1);
    assert_allclose(lstsq_chol(a, a).value(), identity_array<float>(3));
}

void test_lstsq_chol_complex() {
    auto a = uniform_random_complex_array<float>(ArrayShape{5, 3}, 1);
    assert_allclose(lstsq_chol(a, a).value(), identity_array<std::complex<float>>(3));

    auto as = uniform_random_complex_array<float>(ArrayShape{5, 5}, 1);
    auto b = uniform_random_complex_array<float>(ArrayShape{5, 2}, 3);
    assert_allclose(dot(as, lstsq_chol(as, b).value()), b, (float)1e-2);
}

void test_sort_svd() {
    Array<float> u{{1, 2, 3}, {4, 5, 6}};
    Array<float> s{3, 1, 2};
    Array<float> vT{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    sort_svd(u, s, vT);
    assert_allclose(u, Array<float>{{1, 3, 2}, {4, 6, 5}});
    assert_allclose(s, Array<float>{3, 2, 1});
    assert_allclose(vT, Array<float>{{1, 2, 3}, {7, 8, 9}, {4, 5, 6}});
}

void test_outer() {
    Array<float> a = random_array<float>(ArrayShape{3, 4});
    Array<float> b2 = random_array<float>(ArrayShape{1, 4});
    Array<float> b1 = random_array<float>(ArrayShape{4});
    assert_allclose(outer(a, b2).flattened(), dot(a, b1));
    assert_isclose(outer(b2, b2)(0, 0), dot0d(b1, b1));
}

void test_solve_svd() {
    Array<float> a = random_array2<float>(ArrayShape{5, 3}, 1);
    Array<float> uT;
    Array<float> s;
    Array<float> vT;
    svd(a, uT, s, vT);
    assert_allclose(reconstruct_svd(uT.T(), s, vT), a, (float)1e-3);
    assert_allclose(dot(pinv_svd(uT.T(), s, vT), a), identity_array<float>(3), (float)1e-3);
}

void test_solve_svd_complex() {
    Array<std::complex<float>> a = (
        uniform_random_array<std::complex<float>>(ArrayShape{5, 3}, 1) +
        std::complex<float>(0, 1) *
        uniform_random_array<std::complex<float>>(ArrayShape{5, 3}, 2));
    Array<std::complex<float>> uT;
    Array<float> s;
    Array<std::complex<float>> vT;
    svd(a, uT, s, vT);
    assert_allclose(reconstruct_svd(uT.H(), s, vT), a, (float)1e-3);
    assert_allclose(dot(pinv_svd(uT.H(), s, vT), a), identity_array<std::complex<float>>(3), (float)1e-3);
}

void test_cond() {
    Array<double> a = random_array3<double>(ArrayShape{5, 3}, 1);
    assert_isclose(cond(a), 14.3934, 1e-3);
    assert_isclose(cond4(a), 14.3934, 1e-3);
}

void test_approximate_rank() {
    Array<double> a = dot(
        random_array3<double>(ArrayShape{6, 2}, 1),
        random_array3<double>(ArrayShape{2, 5}, 2));
    assert_allclose(approximate_rank(a, 2), a);
    assert_isclose(approximate_rank(a, 1)(0, 0), 1.10765, 1e-3);
}

void test_batch_dot() {
    Array<float> a = uniform_random_array<float>(ArrayShape{3, 4}, 1);
    Array<float> b = uniform_random_array<float>(ArrayShape{4, 5}, 2);
    assert_allclose(dot(a, b), batch_dot(a, b));
}

void test_huber_norm() {
    Array<double> x = linspace<double>(0, 5, 10);
    assert_allclose(
        huber_norm(x, 3., false),
        Array<double>{
            0, 0.0514403, 0.205761, 0.462963, 0.823045,
            1.28601, 1.83333, 2.38889, 2.94444, 3.5},
        1e-5);

    Array<double> x2 = Array<double>({x / std::sqrt(2), x / std::sqrt(2)});
    assert_allclose(
        huber_norm(x2, 3., true),
        Array<double>{
            0, 0.0514403, 0.205761, 0.462963, 0.823045,
            1.28601, 1.83333, 2.38889, 2.94444, 3.5},
        1e-5);
}

void test_sum_axis() {
    Array<float> a{0, 1, 2};
    Array<float> b({a, 4.f * a});
    assert_isclose<float>(sum(a, 0)(), 3);
    assert_allclose<float>(sum(b, 0), Array<float>{0.f, 5.f, 10.f});
    assert_allclose<float>(sum(b, 1), Array<float>{3.f, 12.f});

    Array<float> c = uniform_random_array<float>(ArrayShape{2, 4, 5}, 1);
    assert_allclose<float>(sum(c, 0), c[0] + c[1]);
}

void test_array_array_comparison() {
    assert_allequal(
        Array<float>{1, 3, 2} < Array<float>{2.f, 1.f, 4.f},
        Array<bool>{true, false, true});
    assert_allequal(
        FixedArray<float, 3>{1.f, 3.f, 2.f} < FixedArray<float, 3>{2.f, 1.f, 4.f},
        FixedArray<bool, 3>{true, false, true});
}

void test_interpolate() {
    assert_allclose(
        interpolate(
            Array<float>{0.f, 1.f, 2.f},
            Array<float>{10.f, 10.4f, 10.6f}),
        Array<float>{10.f, 10.4f, 10.6f});
    assert_allclose(
        interpolate(
            Array<float>{-0.1f, 0.5f, 2.1f},
            Array<float>{10.f, 10.4f, 10.6f}),
        Array<float>{NAN, 10.2f, NAN});
}

void test_substitute_nans() {
    Array<float> a{0, 1, 2, NAN, 4, 5, 6};
    assert_allclose(
        substitute_nans(a, -1.f),
        Array<float>{0, 1, 2, -1, 4, 5, 6});
}

void test_power() {
    assert_allclose(
        pow(10.f, Array<float>{0, 1, 2, NAN, 4, 5, 6}),
        Array<float>{1, 10, 100, NAN, 10000, 100000, 1e+6});
    assert_allclose(
        pow(Array<float>{0, 1, 2, NAN, 4, 5, 6}, 10.f),
        Array<float>{0, 1, 1024, NAN, 1.04858e+06, 9.76562e+06, 6.04662e+07},
        1e2);
}

void test_fixed_shape() {
    Array<float> ad = uniform_random_array<float>(ArrayShape{3, 4}, 1);
    FixedArray<float, 3, 4> a{ ad };
    // auto x = rm_last(a);
    {
        constexpr auto b = FixedArrayShape<3, 4, 3>();
        constexpr auto c = FixedArrayShape<3, 4>();
        assert_isequal<size_t>(FasUtils::rows_as_1D(b).nelements(), 3 * 4 * 3);
        assert_isequal<size_t>(FasUtils::rows_as_1D(c).nelements(), 3 * 4);
        assert_isequal<size_t>(FasUtils::columns_as_1D(c).nelements(), 3 * 4);
    }

    {
        Array<float> bd = uniform_random_array<float>(ArrayShape{ 4, 5, 6 }, 2);
        FixedArray<float, 4, 5, 6> b{ bd };
        auto a_dot_b = dot(a, b);  // Workaround for MSVC
        assert_isequal(a_dot_b.ndim(), dot(ad, bd).ndim());
        assert_isclose(
            dot(a, b)(1, 2, 3),
            dot(ad, bd)(1, 2, 3));
    }
}

void test_fixed_outer() {
    Array<float> ad = uniform_random_array<float>(ArrayShape{3, 4}, 1);
    Array<float> bd = uniform_random_array<float>(ArrayShape{6, 5, 4}, 2);
    FixedArray<float, 3, 4> a{ad};
    FixedArray<float, 6, 5, 4> b{bd};

    auto a_outer_b = outer(a, b);  // Workaround for MSVC
    assert_isequal(a_outer_b.ndim(), outer(ad, bd).ndim());
    assert_isclose(
        outer(a, b)(1, 2, 3),
        outer(ad, bd)(1, 2, 3));
}

void test_regularization() {
    auto Jg = uniform_random_array<float>(ArrayShape{6, 3}, 1);
    auto residual = uniform_random_array<float>(ArrayShape{6}, 2);
    auto x0 = uniform_random_array<float>(ArrayShape{3}, 3);
    auto ATA = dot2d(Jg.vH(), Jg);
    auto ATr = dot1d(Jg.vH(), residual);

    Array<float> v1 = x0 + solve_symm_1d(ATA, ATr, float(1e-1), float(1e-1)).value();
    Array<float> v2 = solve_symm_1d(ATA, ATr + dot1d(ATA, x0), float(1e-1), float(1e-1), &x0).value();

    assert_allclose(v1, v2);

    // return x + lstsq_chol_1d(Jg, residual, float(1e-2), float(1e-2));
}

void test_gaussian_elimination() {
    Array<float> a = uniform_random_array<float>(ArrayShape{5, 5}, 2);
    Array<float> b = uniform_random_array<float>(ArrayShape{5}, 2);
    Array<float> x = gaussian_elimination_1d(a, b);
    assert_allclose(dot(a, x), b);

    Array<float> ata = dot2d(a.T(), a);
    Array<float> c = solve_symm_1d(ata, b, 0.1f, 0.2f).value();
    Array<float> d = gaussian_elimination_1d(ata, b, 0.1f, 0.2f);
    assert_allclose(c, d);
}

void test_cg() {
    Array<float> a = uniform_random_array<float>(ArrayShape{5, 5}, 1);
    Array<float> A = dot2d(a.vH(), a);
    Array<float> b = uniform_random_array<float>(ArrayShape{A.shape(0)}, 2);
    Array<float> x = solve_symm_1d(A, b).value();
    Array<float> x0 = x + 10.f * uniform_random_array<float>(x.shape(), 3);
    Array<float> x1 = cg_simple(A, x0, b, 100, float(1e-6));
    assert_allclose(x, x1, (float)1e-2);
}

void test_set_difference() {
    Array<size_t> a{20, 30, 1, 2, 3, 4, 5};
    Array<size_t> b{2, 3, 4, 5, 6, 7};
    assert_allequal(set_difference(a, b), Array<size_t>{1, 20, 30});
}

void test_nonzero_ids() {
    Array<size_t> a{
        {0, 3, 2, 3},
        {2, 2, 0, 5}};
    assert_allequal(
        nonzero_ids(a),
        Array<size_t>{
            {0, 1},
            {0, 2},
            {0, 3},
            {1, 0},
            {1, 1},
            {1, 3}});
}

void test_fixed_transposed() {
    FixedArray<float, 3, 4> a{uniform_random_array<float>(ArrayShape{3, 4}, 1)};
    assert_allclose(a.to_array().T(), a.T().to_array());
}

void test_fixed_cholesky() {
    Array<float> a_ = uniform_random_array<float>(ArrayShape{5, 5}, 1);
    Array<float> b = uniform_random_array<float>(ArrayShape{5}, 2);
    Array<float> a = dot2d(a_.vH(), a_);
    FixedArray<float, 5, 5> fa{a};
    FixedArray<float, 5> fb{b};
    assert_allclose(
        solve_symm_1d(a, b).value(),
        solve_symm_1d(fa, fb).value().to_array());
}

void test_interp() {
    {
        Interp<float> interp{
            {0.f, 0.1f, 2.f, 3.f},
            {4.123f, 2.567f, 3.89f, 4.2f},
            OutOfRangeBehavior::EXPLICIT,
            -10,
            20 };
        assert_allclose(
            Array<float>{interp(-1.f), interp(4.3f)},
            Array<float>{-10, 20});
        assert_allclose(
            Array<float>{interp(0.f), interp(0.05f), interp(0.1f), interp(2.f), interp(3.f)},
            Array<float>{4.123f, 3.345f, 2.567f, 3.89f, 4.2f});
    }
    {
        Interp<float> interp{
            {0.f, 0.1f, 2.f, 3.f},
            {4.123f, 2.567f, 3.89f, 4.2f},
            OutOfRangeBehavior::EXTRAPOLATE,
            -10,
            20 };
        assert_allclose(
            Array<float>{interp(-1.f), interp(4.3f)},
            Array<float>{19.6830024719238281, 4.60299968719482422});
        assert_allclose(
            Array<float>{interp(0.f), interp(0.05f), interp(0.1f), interp(2.f), interp(3.f)},
            Array<float>{4.123f, 3.345f, 2.567f, 3.89f, 4.2f});
    }
}

void test_projection() {
    TransformationMatrix<float, float, 2> a{
        FixedArray<float, 2, 2>{random_array3<float>(ArrayShape{2, 2}, 3)},
        FixedArray<float, 2>{random_array3<float>(ArrayShape{2}, 3)} };
    TransformationMatrix<float, float, 3> b{
        FixedArray<float, 3, 3>{random_array3<float>(ArrayShape{3, 3}, 4)},
        FixedArray<float, 3>{random_array3<float>(ArrayShape{3}, 5)} };
    assert_allclose(
        dot2d(a.affine(), b.semi_affine()).to_array(),
        a.project(b.semi_affine()).to_array());
}

void test_eigen_jacobi() {
    Array<double> m = random_array3<double>(ArrayShape{ 4, 4 }, 1);
    m += m.T();
    m(0, 2);
    Array<double> evals;
    Array<double> evecs;
    eigs_symm(m, evals, evecs);
    assert_allclose(m - reconstruct_svd(evecs.T(), evals, evecs), zeros<double>(ArrayShape{ 4, 4 }));
}

void test_least_common_multiple() {
    std::vector<float> data({1.3f, 2.6f, 0.65f});
    assert_isclose(least_common_multiple(data.begin(), data.end(), 1e-6f, 10'000), 2.6f);
}

void test_quaternion_series() {
    auto now = std::chrono::steady_clock::now();
    auto d0 = std::chrono::steady_clock::duration{ std::chrono::nanoseconds{100} };
    auto d1 = std::chrono::steady_clock::duration{ std::chrono::nanoseconds{110} };
    auto d2 = std::chrono::steady_clock::duration{ std::chrono::nanoseconds{150} };
    auto d3 = std::chrono::steady_clock::duration{ std::chrono::nanoseconds{170} };
    {
        QuaternionSeries<float, double, 3> qs;
        for (size_t i = 0; i < 2; ++i) {
            linfo() << "i=" << i;
            // linfo() << qs.get(now);
            qs.append(OffsetAndQuaternion<float, double>{FixedArray<double, 3>{0.1, 0.2, 0.3}, Quaternion<float>::identity()}, now + d0);
            linfo() << qs.get(now);
            linfo() << qs.get(now + d1);
            qs.append(OffsetAndQuaternion<float, double>{FixedArray<double, 3>{0.5, 0.6, 0.7}, Quaternion<float>::identity()}, now + d2);
            linfo() << qs.get(now);
            linfo() << qs.get(now + d0);
            linfo() << qs.get(now + d1);
            linfo() << qs.get(now + d2);
            linfo() << qs.get(now + d3);
            qs.clear();
        }
    }
}

void test_fixed_sum() {
    auto a = fixed_ones<float, 2, 3, 4>();
    linfo() << sum<0>(a);
    linfo();
    linfo() << sum<1>(a);
    linfo();
    linfo() << sum<2>(a);
    linfo();
    linfo() << mean<0>(a);
}

void test_simd() {
    using S = FixedPointNumber<int32_t, 5>;
    using V = padded_fixed_array_t<S, 3>;
    V a{ (S)1.f, (S)2.f, (S)3.f };
    V b{ (S)10.f, (S)20.f, (S)30.f };
    linfo() << (int)all_le(a, b);
    linfo() << (int)all_ge(a, b);
    linfo() << std::numeric_limits<S>::lowest();
}

void test_fixed_point() {
    double v = 1.234;
    auto fixed = FixedPointNumber<int32_t, (1 << 11)>::from_float_safe(v);
    linfo() << fixed;
}

void test_inv() {
    auto a = FixedArray<float, 4, 4>{ normal_random_array<float>(ArrayShape{ 4, 4 }, 0) };
    auto i = inv(a).value();
    linfo() << "a\n" << a;
    linfo() << "i\n" << i;
    linfo() << "ip\n" << inv_preconditioned_rc(a).value();
    linfo() << "ip\n" << inv_preconditioned_cr(a).value();
}

int main(int argc, const char** argv) {
    try {
        test_blocking_transposed();
        test_svd();
        test_svd_j();
        test_qdq();
        test_det_2x2();
        test_det_3x3();
        test_identity();
        test_lstsq();
        test_lstsq_complex();
        test_lstsq_chol();
        test_lstsq_chol_complex();
        test_sort_svd();
        test_outer();
        test_solve_svd();
        test_solve_svd_complex();
        test_cond();
        test_approximate_rank();
        test_batch_dot();
        test_huber_norm();
        test_sum_axis();
        test_array_array_comparison();
        test_interpolate();
        test_substitute_nans();
        test_power();
        test_fixed_shape();
        test_fixed_outer();
        test_regularization();
        test_gaussian_elimination();
        test_cg();
        test_set_difference();
        test_nonzero_ids();
        test_fixed_transposed();
        test_fixed_cholesky();
        test_interp();
        test_projection();
        test_eigen_jacobi();
        test_least_common_multiple();
        test_quaternion_series();
        test_fixed_sum();
        test_simd();
        test_fixed_point();
        test_inv();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
