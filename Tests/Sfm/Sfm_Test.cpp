#include <Mlib/Assert.hpp>
#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Coordinates_Fixed.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Fixed_Test.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
#include <Mlib/Math/Power_Iteration/Svd.hpp>
#include <Mlib/Math/Rodrigues.hpp>
#include <Mlib/Math/Svd4.hpp>
#include <Mlib/Sfm/Disparity/Traceable_Patch.hpp>
#include <Mlib/Sfm/Draw/Epilines.hpp>
#include <Mlib/Sfm/Frames/Camera_Frame.hpp>
#include <Mlib/Sfm/Homography/Apply_Homography.hpp>
#include <Mlib/Sfm/Homography/Homography_From_Points.hpp>
#include <Mlib/Sfm/Rigid_Motion/Essential_Matrix_To_TR.h>
#include <Mlib/Sfm/Rigid_Motion/Fundamental_Matrix.hpp>
#include <Mlib/Sfm/Rigid_Motion/Initial_Reconstruction2.hpp>
#include <Mlib/Sfm/Rigid_Motion/Normalized_Projection.hpp>
#include <Mlib/Sfm/Rigid_Motion/Projection_To_TR.hpp>
#include <Mlib/Sfm/Rigid_Motion/Synthetic_Scene.hpp>
#include <Mlib/Sfm/Sparse_Bundle/Marginalized_Map.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <iostream>
#include <map>
#include <random>

using namespace Mlib;
using namespace Mlib::Cv;
using namespace Mlib::Sfm;

void test_numerical_differentiation() {
    auto f = [](const Array<float>& x) {
        return dot1d(Array<float>{
            {1.1f, 1.2f},
            {2.1f, 2.2f}}, x);
    };
    assert_allclose(
        numerical_differentiation(f, Array<float>{2, 3}),
        Array<float>{{1.1f, 1.2f}, {2.1f, 2.2f}},
        float{ 1e-3 });
}

void test_project_points() {
    SyntheticScene sc;
    NormalizedProjection np(sc.y);
    // lerr() << p;
    // lerr() << p.shape();
    // lerr() << find_projection_matrices(P, p, true);
    TransformationMatrix<float, float, 2> ki_out = uninitialized;
    find_projection_matrices(
        sc.x,     // x
        np.yn,    // y
        nullptr,  // ki_precomputed
        nullptr,  // kep_initial
        &ki_out); // ki_out
    assert_allclose(
        np.denormalized_intrinsic_matrix(ki_out).affine().to_array(),
        sc.ki.affine().to_array(),
        float{ 1e-2 });
}

void test_unproject_point() {
    size_t index = 0;
    SyntheticScene sc;
    NormalizedProjection np(sc.y);
    //lerr() << sc.y.shape();
    //lerr() << sc.x.shape();
    Array<FixedArray<float, 2>> y_tracked(ArrayShape{sc.y.shape(0)});
    Array<FixedArray<float, 2>> ys_tracked(ArrayShape{sc.y.shape(0)});
    for (size_t f = 0; f < sc.y.shape(0); ++f) {
        y_tracked(f) = np.yn(f, index);
        ys_tracked(f) = sc.y(f, index);
    }
    TransformationMatrix<float, float, 2> kin = uninitialized;
    find_projection_matrices(
        sc.x,    // x
        np.yn,   // y
        nullptr, // ki_precomputed
        nullptr, // kep_initial
        &kin);   // ki_out
    FixedArray<float, 3> x = reconstructed_point_(y_tracked, kin, sc.ke, nullptr, false, true);
    FixedArray<float, 3> xw = reconstructed_point_reweighted(y_tracked, kin, sc.ke);
    FixedArray<float, 3> xs = reconstructed_point_(ys_tracked, sc.ki, sc.ke,
        nullptr, // weights
        false,   // method2
        false);  // points_are_normalized
    //lerr() << "x " << x;
    //lerr() << "sc.c() " << sc.x[index].row_range(0, 3);
    assert_allclose(x.to_array(), sc.x(index).to_array(), float{ 1e-3 });
    assert_allclose(xw.to_array(), sc.x(index).to_array(), float{ 1e-3 });
    assert_allclose(xs.to_array(), sc.x(index).to_array(), float{ 1e-3 });
}

void test_initial_reconstruction() {
    SyntheticScene sc(true); // true = zero_first_extrinsic
    NormalizedProjection np(sc.y);
    // Array<float> ir3 = initial_reconstruction_x3(sc.R(0, 1), sc.t2(0, 1), sc.ki, sc.y[0], sc.y[1]);
    // lerr() << sc.x / sc.x(0, 2);
    // lerr() << ir3 / ir3(0, 1);
    // lerr() << ir3;

    // this did not work without normalization
    Array<FixedArray<float, 3>> irX = initial_reconstruction(
        TransformationMatrix<float, float, 3>{
            sc.dR(0, 1),
            sc.dt2(0, 1)}.inverted(),
        np.normalized_intrinsic_matrix(sc.ki),
        np.yn[0],
        np.yn[1]);
    assert_allclose(
        Array<float>{ irX } / irX(3)(1),
        Array<float>{ sc.x } / sc.x(3)(1),
        float{ 2e-2 });
    // lerr() << "x\n" << irX/irX(0, 2);
}

void test_known_ki_alignment() {
    SyntheticScene sc;
    NormalizedProjection np(sc.y);
    TransformationMatrix<float, float, 2> kin = uninitialized;
    find_projection_matrices(
        sc.x,     // x
        np.yn,    // y
        nullptr,  // ki_precomputed
        nullptr,  // kep_initial
        &kin);    // ki_out
    Array<TransformationMatrix<float, float, 3>> ke;
    find_projection_matrices(
        sc.x,     // x
        np.yn,    // y
        &kin,     // ki_precomputed
        nullptr,  // kep_initial
        nullptr,  // ki_out
        &ke);     // ke_out
    assert_allclose(
        Array<float>{ke.applied<FixedArray<float, 4, 4>>([](const auto& x) {return x.affine(); })},
        Array<float>{sc.ke.applied<FixedArray<float, 4, 4>>([](const auto& x) {return x.affine(); })},
        float{ 1e-5 });

    TransformationMatrix<float, float, 2> kin2 = np.normalized_intrinsic_matrix(sc.ki);
    assert_allclose(kin.affine().to_array(), kin2.affine().to_array(), float{ 1e-5 });

    Array<TransformationMatrix<float, float, 3>> ke2;
    Array<FixedArray<float, 3>> x_out;
    find_projection_matrices_twopass(
        sc.x,       // x
        np.yn,      // y
        &kin,       // ki_precomputed
        // nullptr,    // kep_initial
        nullptr,    // ki_out
        &ke2,       // ke_out
        nullptr,    // kep_out
        &x_out);    // x_out
    // (b)undle, (s)ynthetic
    // b * a = s, a = b \ s
    FixedArray<float, 4, 4> a = lstsq_chol(ke2(0).affine(), sc.ke(0).affine()).value();
    // lerr() << ke2[0];
    // lerr() << sc.ke[0];
    assert_allclose(
        dot(ke2(0).affine(), a).to_array(),
        sc.ke(0).affine().to_array(),
        float{ 1e-5 });
    assert_allclose(
        dot(ke2(1).affine(), a).to_array(),
        sc.ke(1).affine().to_array(),
        1e-4f);
}

struct HomographyData {
    HomographyData()
    :y0(Array<float>::from_dynamic<2>(Array<float>({
        random_array2<float>(ArrayShape{20}, 1),
        random_array2<float>(ArrayShape{20}, 2)}).T())),
     y1(y0.shape()),
     homography{FixedArray<float, 3, 3>::init(
        0.5f, 0.6f, 0.2f,
        0.1f, 0.7f, 0.15f,
        0.25f, 0.9f, 1.f)} // last element must be 1
    {
        for (size_t r = 0; r < y1.shape(0); ++r) {
            y1(r) = apply_homography(homography, y0(r));
        }
    }
    Array<FixedArray<float, 2>> y0;
    Array<FixedArray<float, 2>> y1;
    FixedArray<float, 3, 3> homography;
};

void test_find_fundamental_matrix_homography() {
    HomographyData hd;
    //lerr() << y0;
    //lerr() << y1;

    FixedArray<float, 3, 3> F = find_fundamental_matrix(hd.y0, hd.y1);
    //lerr() << F;
    //inverse iteration:
    //assert_allclose(F, Array<float>{{-0.0499249, -0.560293, 0.328151},
    //                                {0.476451, 0.372055, -0.44176},
    //                                {-0.0907307, 0.0841532, 0.000316865}});
    assert_allclose<float>(
        fundamental_error(F, hd.y0, hd.y1),
        zeros<float>(ArrayShape{hd.y0.shape(0)}),
        float(1e-7));
}

void test_find_fundamental_matrix_synthetic_scene() {
    SyntheticScene sc;
    NormalizedProjection np(sc.y);
    // lerr() << sc.y.shape();
    FixedArray<float, 3, 3> Fn = find_fundamental_matrix(np.yn[0], np.yn[1]);
    FixedArray<float, 3, 3> F = find_fundamental_matrix(sc.y[0], sc.y[1]);
    //lerr() << fundamental_error(Fn, np.yn[0], np.yn[1]);
    //lerr() << fundamental_error(F, sc.y[0], sc.y[1]);
    assert_allclose<float>(
        fundamental_error(Fn, np.yn[0], np.yn[1]),
        zeros<float>(ArrayShape{np.yn.shape(1)}),
        float(1e-5));
    assert_allclose<float>(
        fundamental_error(F, sc.y[0], sc.y[1]),
        zeros<float>(ArrayShape{sc.y.shape(1)}),
        float(1e-2));
}

void test_synthetic_scene() {
    SyntheticScene sc;
    sc.draw_to_bmp("TestOut/scene-0.bmp", 0, 1);
    sc.draw_to_bmp("TestOut/scene-1.bmp", 1, 2);
    sc.draw_to_bmp("TestOut/scene-2.bmp", 2, 3);
    sc.draw_to_bmp("TestOut/scene-3.bmp", 3, 4);
}

void test_find_essential_matrix() {
    size_t i0 = 0;
    size_t i1 = 1;
    SyntheticScene sc;
    NormalizedProjection np(sc.y);
    // lerr() << sc.y.shape();
    FixedArray<float, 3, 3> Fn = find_fundamental_matrix(np.yn[i0], np.yn[i1]);
    FixedArray<float, 3, 3> F = find_fundamental_matrix(sc.y[i0], sc.y[i1]);
    assert_allclose(
        fundamental_error(Fn, np.yn[i0], np.yn[i1]),
        zeros<float>(ArrayShape{np.yn[i0].shape(0)}));
    assert_allclose(
        fundamental_error(F, sc.y[i0], sc.y[i1]),
        zeros<float>(ArrayShape{sc.y[i0].shape(0)}),
        float{ 1e-2 });
    // lerr() << F;
    // "denormalized_intrinsic_matrix" is not used,
    // both E and y are normalized.
    TransformationMatrix<float, float, 2> kin = uninitialized;
    find_projection_matrices(
        sc.x,      // x
        np.yn,     // y
        nullptr,   // ki_precomputed
        nullptr,   // kep_initial
        &kin);     // ki_out
    //lerr() << "kin\n" << kin;
    //Array<float> E = fundamental_to_essential(Fn, kin);
    ProjectionToTR ptr(np.yn[i0], np.yn[i1], kin, { 0.f, float{INFINITY} });
    //lerr() << "t " << e2tr.t;
    //lerr() << "R\n" << e2tr.R;
    //lerr() << "ke[i0]\n" << sc.ke[i0];
    //lerr() << "ke[i1]\n" << sc.ke[i1];
    //position is missing
    //lerr() << "diff\n" << lstsq_chol(sc.ke[i1].T(), sc.ke[i0].T());
    //lerr() << "dke\n" << sc.delta_ke(i0, i1);
    //lerr() << "t " << sc.t(i0, i1);
    //lerr() << "R\n" << sc.R(i0, i1);
    assert_allclose(ptr.ke.inverted().t.to_array(), sc.dt2(i0, i1).to_array(), 1e-4f);
    assert_allclose(ptr.ke.inverted().R.to_array(), sc.dR(i0, i1).to_array(), 1e-4f);
}

void test_projection_to_TR() {
    size_t i0 = 0;
    size_t i1 = 1;
    SyntheticScene sc(true); // true = zero_first_extrinsic
    NormalizedProjection np(sc.y);
    ProjectionToTR ptr(np.yn[i0], np.yn[i1], np.normalized_intrinsic_matrix(sc.ki), { 0.f, float{INFINITY} });
    //lerr() << "t " << ptr.t;
    //lerr() << "R0\n" << sc.R(i0, i1);
    //lerr() << "t0 " << sc.t2(i0, i1);
    assert_allclose(ptr.ke.inverted().t.to_array(), sc.dt2(i0, i1).to_array(), float{ 1e-3 });
    assert_allclose(ptr.ke.inverted().R.to_array(), sc.dR(i0, i1).to_array(), 1e-4f);
    Array<float> u{ ptr.initial_reconstruction().reconstructed() };
    Array<float> x{ sc.x };
    assert_allclose(u / mean(abs(u)), x / mean(abs(x)), float{ 2e-1 });
}

void test_fundamental_from_TR() {
    size_t i0 = 0;
    size_t i1 = 1;
    SyntheticScene sc(
        true,   // true = zero_first_extrinsic
        10);    // tR_multiplier
    FixedArray<float, 3, 3> F = find_fundamental_matrix(sc.y[0], sc.y[1]);
    FixedArray<float, 3, 3> Ftr = fundamental_from_camera(sc.ki, sc.ki, sc.delta_ke(i0, i1));
    assert_allclose(F.to_array() / F(2, 2), Ftr.to_array() / Ftr(2, 2), float{ 1e-5 });
}

void test_find_homography() {
    HomographyData hd;
    FixedArray<float, 3, 3> h = homography_from_points(hd.y0, hd.y1);
    assert_allclose(h.to_array(), hd.homography.to_array(), 1e-4f);
}

void test_svd_easy_matrix() {
    Array<float> E{
        {5.f, 0.0627702f, 0.514959f},
        {0.0627705f, 5.f, 0.480495f},
        {0.514955f, 0.480504f, 4.f}};
    Array<float> uT;
    Array<float> s;
    Array<float> vT;
    svd(E, uT, s, vT);
    assert_allclose(outer(uT, uT), identity_array<float>(3), float{ 1e-3 });
    assert_allclose(outer(vT, vT), identity_array<float>(3), float{ 1e-3 });
}

void test_svd_essential_matrix() {
    Array<double> E{
        {3.94342e-07, 0.0627702, -0.514959},
        {-0.0627705, 4.19942e-07, 0.480495},
        {0.514955, -0.480504, 7.52657e-05}};
    Array<double> u;
    Array<double> s;
    Array<double> vT;

    svd4(E, u, s, vT);
    assert_allclose(outer(u, u), identity_array<double>(3));
    assert_allclose(outer(vT, vT), identity_array<double>(3));
}

void test_svd_essential_matrix2() {
    Array<double> E{
        {-0.00230552, -0.454297, 0.200921},
        {0.448298, -0.00522601, -1.50538},
        {-0.189861, 1.50485, -0.00129056}};

    Array<double> u_ref = Array<double>{
        {-0.25851, -0.17774, 0.94952},
        {0.83392, -0.53719, 0.12648},
        {0.48759, 0.82452, 0.28709}};
    Array<double> s_ref = Array<double>{1.5834e+00, 1.5834e+00, 6.5975e-06};
    Array<double> vT_ref = Array<double>{
        {0.17801, -0.25069, 0.95156},
        {0.53480, 0.83637, 0.12030},
        {-0.82601, 0.48748, 0.28295}}.T();

    Array<double> u;
    Array<double> s;
    Array<double> vT;
    svd4(E, u, s, vT);
    // lerr() << "u\n" << u;
    // lerr() << "s " << s;
    // lerr() << "v\n" << vT.T();

    assert_allclose(abs(u), abs(u_ref), 1e-4f);
    assert_allclose(s, s_ref, 1e-4f);
    assert_allclose(abs(vT), abs(vT_ref), 1e-4f);
    assert_allclose(reconstruct_svd(u, s, vT), E);
}

Array<float> to_rgb(const Array<float>& a) {
    return Array<float>({a, 0.1f * a, 0.2f * a});
}

void test_traceable_patch() {
    const Array<float> image = to_rgb(Array<float>{
        {1, 2, 3, 4, 5},
        {6, 7, 8, 9, 10},
        {11, 12, 13, 14, 15}});
    const FixedArray<size_t, 2> patch_center{ 1u, 2u };
    TraceablePatch p(image, patch_center, FixedArray<size_t, 2>{3u, 4u}, FixedArray<size_t, 2>{0u, 0u}, 5);
    assert_allclose(
        p.image_patch_,
        to_rgb(Array<float>{
            {1, 2, 3, 4},
            {6, 7, 8, 9},
            {11, 12, 13, 14}}));
    const Array<float> new_image = to_rgb(Array<float>{
        {5, 5, 5, 1, 2, 3, 4, 5},
        {5, 5, 5, 6, 7, 8, 9, 10},
        {5, 5, 5, 11, 12, 13, 14, 15}});
    assert_isclose<float>(p.error_at_position(new_image, FixedArray<size_t, 2>{1u, 5u}), 0);
    assert_isclose<float>(p.error_at_position(image, FixedArray<size_t, 2>{1u, 2u}), 0);
    assert_allequal(
        p.new_position_in_box(new_image, patch_center, FixedArray<size_t, 2>{10u, 10u}, 0.002f),
        FixedArray<size_t, 2>{1u, 5u});
}

void test_traceable_patch_nan() {
    const Array<float> image = to_rgb(Array<float>{
        {1, 2, 3, 4, 5},
        {6, 7, 8, 9, 10},
        {11, 12, 13, 14, 15}});
    const FixedArray<size_t, 2> patch_center{1u, 2u};
    TraceablePatch p(image, patch_center, FixedArray<size_t, 2>{3u, 4u}, FixedArray<size_t, 2>{1u, 1u});
    assert_allclose(
        p.image_patch_,
        to_rgb(Array<float>{
            {1, 2, 3, 4},
            {6, 7, NAN, 9},
            {11, 12, 13, 14}}));
}

void test_camera_frame() {
    const FixedArray<float, 3> x{ random_array2<float>(ArrayShape{3}, 1) };
    CameraFrame cf(
        TransformationMatrix<float, float, 3>{
            tait_bryan_angles_2_matrix<float>(FixedArray<float, 3>{0.1f, 0.2f, 0.3f}),
            FixedArray<float, 3>{random_array2<float>(ArrayShape{ 3 }, 1)}});
    const FixedArray<float, 3> y = cf.projection_matrix_3x4().transform(x);
    const FixedArray<float, 3> x1 = cf.reconstruction_matrix_3x4().transform(y);
    assert_allclose(x.to_array(), x1.to_array());
}

void test_find_epiline() {
    const Array<float> y0 = random_array2<float>(ArrayShape{20, 2}, 1);
    const Array<float> tmp = y0.T();
    tmp[0] += 0.1f;
    tmp[0] += 0.1f * random_array2<float>(ArrayShape{y0.shape(0)}, 1);
    tmp[1] += 0.01f * random_array2<float>(ArrayShape{y0.shape(0)}, 1);
    const Array<float> y1 = tmp.T();
    const FixedArray<float, 3, 3> F = find_fundamental_matrix(Array<float>::from_dynamic<2>(y0), Array<float>::from_dynamic<2>(y1));
    //lerr() << fundamental_error(F, y0, y1);
    //lerr() << F;
    StbImage3 bmp{FixedArray<size_t, 2>{200u, 200u}, Rgb24::white()};
    highlight_features(Array<float>::from_dynamic<2>(y0 * 180.f), bmp, 2, Rgb24::red());
    highlight_features(Array<float>::from_dynamic<2>(y1 * 180.f), bmp, 2, Rgb24::blue());
    FixedArray<float, 2> p = uninitialized;
    FixedArray<float, 2> v = uninitialized;
    find_epiline(F, FixedArray<float, 2>{ y0[1] }, p, v);
    assert_allclose(p.to_array(), Array<float>{0.784089f, 0.788539f});
    assert_allclose(v.to_array(), Array<float>{0.703581f, 0.070358f});
    //lerr() << "v " << v;
    //lerr() << (F, y0[0]);
    //lerr() << (F.T(), y1[0]);
    //lerr() << (y1[0], (F, y0[0]))();
    draw_epilines_from_F(F, bmp, Rgb24::green());
    draw_inverse_epilines_from_F(F, bmp, Rgb24::blue(), 72);
    bmp.draw_infinite_line(
        a2fi(p * 180.f),
        a2fi((p + v) * 180.f),
        0,
        Rgb24::black());
    bmp.save_to_file("TestOut/epiline.png");
}

/**
 * Not working, only works around point 0.
 * Using axis-angle representation instead,
 * where only the angle varies.
 */
// void test_rodrigues_jacobian() {
//     {
//         Array<float> k = zeros<float>(ArrayShape{3});
//         Array<float> x = random_array3<float>(ArrayShape{3}, 3);
//         assert_allclose(
//             numerical_differentiation([&](
//                 const Array<float>& kk){ return (rodrigues(kk), x); },
//                 k,
//                 float(1e-3)),
//             rodrigues_jacobian_dk(k, x),
//             float(1e-3));
//     }
//
//     {
//         Array<double> k = random_array3<double>(ArrayShape{3}, 2);
//         Array<double> x = random_array3<double>(ArrayShape{3}, 3);
//         for (double i = 1e-3; i > 1e-13; i/=1.5) {
//             lerr() << numerical_differentiation([&](
//                     const Array<double>& kk){ return (rodrigues(kk), x); },
//                     k,
//                     double(i));
//         }
//         lerr() << rodrigues_jacobian_dk(k, x);
//     }
// }

void test_marginalized_map() {
    {
        MarginalizedMap<std::map<std::string, int>> mm;
        mm.active_["a"] = 5;
        size_t ncalls = 0;
        for (const auto& c : mm) {
            ++ncalls;
            assert_true(c.it_->first == "a");
            assert_true(c.it_->second == 5);
            assert_true(c.state_ == MmState::ACTIVE);

            assert_true(c.first == "a");
            assert_true(c.second == 5);
        }
        assert_true(ncalls == 1);
        mm.move_to_linearized("a");
        assert_true(mm.linearized_.find("a") != mm.linearized_.end());
        mm.move_to_marginalized("a");
        assert_true(mm.marginalized_.find("a") != mm.marginalized_.end());
    }
    {
        MarginalizedMap<std::map<std::string, int>> mm;
        mm.marginalized_["a"] = 5;
        size_t ncalls = 0;
        for (const auto& c : mm) {
            ++ncalls;
            assert_true(c.it_->first == "a");
            assert_true(c.it_->second == 5);
            assert_true(c.state_ == MmState::MARGINALIZED);
        }
        assert_true(ncalls == 1);
    }

    {
        MarginalizedMap<std::map<std::string, int>> mm;
        mm.active_["a"] = 5;
        auto const& mm1 = mm;
        size_t ncalls = 0;
        for (const auto& c : mm1) {
            ++ncalls;
            assert_true(c.it_->first == "a");
            assert_true(c.it_->second == 5);
            assert_true(c.state_ == MmState::ACTIVE);
        }
        assert_true(ncalls == 1);
    }
    {
        MarginalizedMap<std::map<std::string, int>> mm;
        mm.marginalized_["a"] = 5;
        auto const& mm1 = mm;
        size_t ncalls = 0;
        for (const auto& c : mm1) {
            ++ncalls;
            assert_true(c.it_->first == "a");
            assert_true(c.it_->second == 5);
            assert_true(c.state_ == MmState::MARGINALIZED);
        }
        assert_true(ncalls == 1);
    }
    {
        MarginalizedMap<std::map<std::string, int>> mm;
        mm.marginalized_["a"] = 5;
        auto const& mm1 = mm;
        size_t ncalls = 0;
        for (auto it = mm1.rbegin(); it != mm1.rend(); ++it) {
            ++ncalls;
            assert_true(it->it_->first == "a");
            assert_true(it->it_->second == 5);
            assert_true(it->state_ == MmState::MARGINALIZED);
        }
        assert_true(ncalls == 1);
    }
    {
        MarginalizedMap<std::map<std::string, int>> mm;
        auto const& mm1 = mm;
        mm.marginalized_["a"] = 5;
        mm.marginalized_["x"] = 7;
        {
            auto it0 = mm.find("b");
            assert_true(it0 == mm.end());
            auto it = mm.find("x");
            assert_true(it->it_->second == 7);
            --it;
            assert_true(it->it_->second == 5);
            ++it;
            assert_true(it->it_->second == 7);
            assert_true(it != mm.end());
            ++it;
            assert_true(it == mm.end());
        }
        mm.linearized_["y"] = 10;
        mm.active_["z"] = 9;
        {
            std::vector<int> l;
            for (auto it = mm.rbegin(); it != mm.rend(); ++it) {
                l.push_back(it->it_->second);
            }
            assert_true(l[0] == 9);
            assert_true(l[1] == 10);
            assert_true(l[2] == 7);
            assert_true(l[3] == 5);
        }
        {
            std::vector<int> l;
            for (const auto& c : mm) {
                l.push_back(c.it_->second);
            }
            assert_true(l[0] == 5);
            assert_true(l[1] == 7);
            assert_true(l[2] == 10);
            assert_true(l[3] == 9);
        }
        {
            std::vector<int> l;
            for (auto it = mm1.rbegin(); it != mm1.rend(); ++it) {
                l.push_back(it->it_->second);
            }
            assert_true(l[0] == 9);
            assert_true(l[1] == 10);
            assert_true(l[2] == 7);
            assert_true(l[3] == 5);
        }
        {
            std::vector<int> l;
            for (const auto& c : mm1) {
                l.push_back(c.it_->second);
            }
            assert_true(l[0] == 5);
            assert_true(l[1] == 7);
            assert_true(l[2] == 10);
            assert_true(l[3] == 9);
        }
    }
}

int main(int argc, char** argv) {
    try {
        test_marginalized_map();
        test_numerical_differentiation();
        test_project_points();
        test_unproject_point();
        test_initial_reconstruction();
        test_synthetic_scene();
        test_find_fundamental_matrix_homography();
        test_find_fundamental_matrix_synthetic_scene();
        test_find_essential_matrix();
        test_find_homography();
        test_svd_easy_matrix();
        test_svd_essential_matrix();
        test_svd_essential_matrix2();
        test_traceable_patch();
        test_traceable_patch_nan();
        test_known_ki_alignment();
        test_projection_to_TR();
        test_fundamental_from_TR();
        test_camera_frame();
        test_find_epiline();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
