#include <Mlib/Cv/Project_Points.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Images/Coordinates.hpp>
#include <Mlib/Images/Features.hpp>
#include <Mlib/Images/PpmImage.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Math/Optimize/Numerical_Differentiation.hpp>
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
        1e-3);
}

void test_project_points() {
    SyntheticScene sc;
    NormalizedProjection np(sc.y);
    // std::cerr << p << std::endl;
    // std::cerr << p.shape() << std::endl;
    // std::cerr << find_projection_matrices(P, p, true) << std::endl;
    Array<float> ki_out;
    find_projection_matrices(
        sc.x,     // x
        np.yn,    // y
        nullptr,  // ki_precomputed
        nullptr,  // kep_initial
        &ki_out); // ki_out
    assert_allclose(
        np.denormalized_intrinsic_matrix(ki_out),
        sc.ki,
        1e-2);
}

void test_unproject_point() {
    size_t index = 0;
    SyntheticScene sc;
    NormalizedProjection np(sc.y);
    //std::cerr << sc.y.shape() << std::endl;
    //std::cerr << sc.x.shape() << std::endl;
    Array<float> y_tracked(ArrayShape{sc.y.shape(0), 3});
    Array<float> ys_tracked(ArrayShape{sc.y.shape(0), 3});
    for(size_t f = 0; f < sc.y.shape(0); ++f) {
        y_tracked[f] = np.yn[f][index];
        ys_tracked[f] = sc.y[f][index];
    }
    Array<float> kin;
    find_projection_matrices(
        sc.x,    // x
        np.yn,   // y
        nullptr, // ki_precomputed
        nullptr, // kep_initial
        &kin);   // ki_out
    Array<float> x = reconstructed_point(y_tracked, kin, sc.ke, nullptr, nullptr, false, true);
    Array<float> xw = reconstructed_point_reweighted(y_tracked, kin, sc.ke);
    Array<float> xs = reconstructed_point(ys_tracked, sc.ki, sc.ke,
        nullptr, // weights
        nullptr, // fs
        false,   // method2
        false);  // points_are_normalized
    //std::cerr << "x " << x << std::endl;
    //std::cerr << "sc.c() " << sc.x[index].row_range(0, 3) << std::endl;
    assert_allclose(x, sc.x[index].row_range(0, 3), 1e-3);
    assert_allclose(xw, sc.x[index].row_range(0, 3), 1e-3);
    assert_allclose(xs, sc.x[index].row_range(0, 3), 1e-3);
}

void test_initial_reconstruction() {
    SyntheticScene sc(true); // true = zero_first_extrinsic
    NormalizedProjection np(sc.y);
    // Array<float> ir3 = initial_reconstruction_x3(sc.R(0, 1), sc.t2(0, 1), sc.ki, sc.y[0], sc.y[1]);
    // std::cerr << sc.x / sc.x(0, 2) << std::endl;
    // std::cerr << ir3 / ir3(0, 1) << std::endl;
    // std::cerr << ir3 << std::endl;

    // this did not work without normalization
    Array<float> irX = initial_reconstruction(
        sc.dR(0, 1),
        sc.dt2(0, 1),
        np.normalized_intrinsic_matrix(sc.ki),
        np.yn[0],
        np.yn[1]);
    assert_allclose(
        irX / irX(3, 1),
        sc.x.col_range(0, 3) / sc.x(3, 1),
        2e-2);
    // std::cerr << "x\n" << irX/irX(0, 2) << std::endl;
}

void test_known_ki_alignment() {
    SyntheticScene sc;
    NormalizedProjection np(sc.y);
    Array<float> kin;
    find_projection_matrices(
        sc.x,     // x
        np.yn,    // y
        nullptr,  // ki_precomputed
        nullptr,  // kep_initial
        &kin);    // ki_out
    Array<float> ke;
    find_projection_matrices(
        sc.x,     // x
        np.yn,    // y
        &kin,     // ki_precomputed
        nullptr,  // kep_initial
        nullptr,  // ki_out
        &ke);     // ke_out
    assert_allclose(ke, sc.ke, 1e-5);

    Array<float> kin2 = np.normalized_intrinsic_matrix(sc.ki);
    assert_allclose(kin, kin2, 1e-5);

    Array<float> ke2;
    Array<float> x_out;
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
    Array<float> a = lstsq_chol(homogenized_4x4(ke2[0]), homogenized_4x4(sc.ke[0]));
    // std::cerr << ke2[0] << std::endl;
    // std::cerr << sc.ke[0] << std::endl;
    assert_allclose(
        dot(homogenized_4x4(ke2[0]), a),
        homogenized_4x4(sc.ke[0]),
        1e-5);
    assert_allclose(
        dot(homogenized_4x4(ke2[1]), a),
        homogenized_4x4(sc.ke[1]),
        1e-4);
}

struct HomographyData {
    HomographyData()
    :y0(Array<float>({
        random_array2<float>(ArrayShape{20}, 1),
        random_array2<float>(ArrayShape{20}, 2),
        ones<float>(ArrayShape{20})}).T()),
     y1(y0.shape()),
     homography{
        {0.5, 0.6, 0.2},
        {0.1, 0.7, 0.15},
        {0.25, 0.9, 1}} // last element must be 1
    {
        for(size_t r = 0; r < y1.shape(0); ++r) {
            y1[r] = apply_homography(homography, y0[r]);
        }
    }
    Array<float> y0;
    Array<float> y1;
    Array<float> homography;
};

void test_find_fundamental_matrix_homography() {
    HomographyData hd;
    //std::cerr << y0 << std::endl;
    //std::cerr << y1;

    Array<float> F = find_fundamental_matrix(hd.y0, hd.y1);
    //std::cerr << F << std::endl;
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
    // std::cerr << sc.y.shape() << std::endl;
    Array<float> Fn = find_fundamental_matrix(np.yn[0], np.yn[1]);
    Array<float> F = find_fundamental_matrix(sc.y[0], sc.y[1]);
    //std::cerr << fundamental_error(Fn, np.yn[0], np.yn[1]) << std::endl;
    //std::cerr << fundamental_error(F, sc.y[0], sc.y[1]) << std::endl;
    //assert_allclose<float>(
    //    fundamental_error(Fn, np.yn[0], np.yn[1]),
    //    zeros<float>(ArrayShape{np.yn.shape(1)}),
    //    float(20));
    assert_allclose<float>(
        fundamental_error(F, sc.y[0], sc.y[1]),
        zeros<float>(ArrayShape{sc.y.shape(1)}),
        float(1e-4));
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
    // std::cerr << sc.y.shape() << std::endl;
    Array<float> Fn = find_fundamental_matrix(np.yn[i0], np.yn[i1]);
    Array<float> F = find_fundamental_matrix(sc.y[i0], sc.y[i1]);
    assert_allclose(
        fundamental_error(Fn, np.yn[i0], np.yn[i1]),
        zeros<float>(ArrayShape{np.yn[i0].shape(0)}));
    assert_allclose(
        fundamental_error(F, sc.y[i0], sc.y[i1]),
        zeros<float>(ArrayShape{sc.y[i0].shape(0)}),
        1e-4);
    // std::cerr << F << std::endl;
    // "denormalized_intrinsic_matrix" is not used,
    // both E and y are normalized.
    Array<float> kin;
    find_projection_matrices(
        sc.x,      // x
        np.yn,     // y
        nullptr,   // ki_precomputed
        nullptr,   // kep_initial
        &kin);     // ki_out
    //std::cerr << "kin\n" << kin << std::endl;
    //Array<float> E = fundamental_to_essential(Fn, kin);
    ProjectionToTR ptr(np.yn[i0], np.yn[i1], kin, 0);
    //std::cerr << "t " << e2tr.t << std::endl;
    //std::cerr << "R\n" << e2tr.R << std::endl;
    //std::cerr << "ke[i0]\n" << sc.ke[i0] << std::endl;
    //std::cerr << "ke[i1]\n" << sc.ke[i1] << std::endl;
    //position is missing
    //std::cerr << "diff\n" << lstsq_chol(sc.ke[i1].T(), sc.ke[i0].T()) << std::endl;
    //std::cerr << "dke\n" << sc.delta_ke(i0, i1) << std::endl;
    //std::cerr << "t " << sc.t(i0, i1) << std::endl;
    //std::cerr << "R\n" << sc.R(i0, i1) << std::endl;
    assert_allclose(ptr.t, sc.dt2(i0, i1), 1e-4);
    assert_allclose(ptr.R, sc.dR(i0, i1), 1e-4);
}

void test_projection_to_TR() {
    size_t i0 = 0;
    size_t i1 = 1;
    SyntheticScene sc(true); // true = zero_first_extrinsic
    NormalizedProjection np(sc.y);
    ProjectionToTR ptr(np.yn[i0], np.yn[i1], np.normalized_intrinsic_matrix(sc.ki), 0);
    //std::cerr << "t " << ptr.t << std::endl;
    //std::cerr << "R0\n" << sc.R(i0, i1) << std::endl;
    //std::cerr << "t0 " << sc.t2(i0, i1) << std::endl;
    assert_allclose(ptr.t, sc.dt2(i0, i1), 1e-3);
    assert_allclose(ptr.R, sc.dR(i0, i1), 1e-4);
    Array<float> u_scaled = ptr.initial_reconstruction().reconstructed() / mean(abs(ptr.initial_reconstruction().reconstructed()));
    Array<float> x_scaled = sc.x / mean(abs(sc.x));
    assert_allclose(u_scaled, x_scaled.col_range(0, 3), 2e-1);
}

void test_fundamental_from_TR() {
    size_t i0 = 0;
    size_t i1 = 1;
    SyntheticScene sc(
        true,   // true = zero_first_extrinsic
        10);    // tR_multiplier
    Array<float> F = find_fundamental_matrix(sc.y[0], sc.y[1]);
    Array<float> Ftr = fundamental_from_camera(sc.ki, sc.ki, sc.dR(i0, i1), sc.dt(i0, i1));
    assert_allclose(F / F(2, 2), Ftr / Ftr(2, 2), 1e-5);
}

void test_find_homography() {
    HomographyData hd;
    Array<float> h = homography_from_points(hd.y0, hd.y1);
    assert_allclose(h, hd.homography, 1e-4);
}

void test_svd_easy_matrix() {
    Array<float> E{
        {5, 0.0627702, 0.514959},
        {0.0627705, 5, 0.480495},
        {0.514955, 0.480504, 4}};
    Array<float> uT;
    Array<float> s;
    Array<float> vT;
    svd(E, uT, s, vT);
    assert_allclose(outer(uT, uT), identity_array<float>(3), 1e-3);
    assert_allclose(outer(vT, vT), identity_array<float>(3), 1e-3);
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
    // std::cerr << "u\n" << u << std::endl;
    // std::cerr << "s " << s << std::endl;
    // std::cerr << "v\n" << vT.T() << std::endl;

    assert_allclose(abs(u), abs(u_ref), 1e-4);
    assert_allclose(s, s_ref, 1e-4);
    assert_allclose(abs(vT), abs(vT_ref), 1e-4);
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
    const ArrayShape patch_center{1, 2};
    TraceablePatch p(image, patch_center, ArrayShape{3, 4}, ArrayShape{0, 0}, 5);
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
    assert_isclose<float>(p.error_at_position(new_image, ArrayShape{1, 5}), 0);
    assert_isclose<float>(p.error_at_position(image, ArrayShape{1, 2}), 0);
    assert_shape_equals(
        p.new_position_in_box(new_image, patch_center, ArrayShape{10, 10}, 0.002),
        ArrayShape{1, 5});
}

void test_traceable_patch_nan() {
    const Array<float> image = to_rgb(Array<float>{
        {1, 2, 3, 4, 5},
        {6, 7, 8, 9, 10},
        {11, 12, 13, 14, 15}});
    const ArrayShape patch_center{1, 2};
    TraceablePatch p(image, patch_center, ArrayShape{3, 4}, ArrayShape{1, 1});
    assert_allclose(
        p.image_patch_,
        to_rgb(Array<float>{
            {1, 2, 3, 4},
            {6, 7, NAN, 9},
            {11, 12, 13, 14}}));
}

void test_camera_frame() {
    const Array<float> x = random_array2<float>(ArrayShape{3}, 1);
    CameraFrame cf(
        tait_bryan_angles_2_matrix<float>(Array<float>{0.1f, 0.2f, 0.3f}),
        random_array2<float>(ArrayShape{3}, 1),
        CameraFrame::undefined_kep);
    const Array<float> y = dot1d(cf.projection_matrix_3x4(), homogenized_4(x));
    const Array<float> x1 = dot1d(cf.reconstruction_matrix_3x4(), homogenized_4(y));
    assert_allclose(x, x1);
}

void test_find_epiline() {
    const Array<float> y0 = homogenized_Nx3(random_array2<float>(ArrayShape{20, 2}, 1));
    const Array<float> tmp = y0.T();
    tmp[0] += 0.1;
    tmp[0] += 0.1f * random_array2<float>(ArrayShape{y0.shape(0)}, 1);
    tmp[1] += 0.01f * random_array2<float>(ArrayShape{y0.shape(0)}, 1);
    const Array<float> y1 = tmp.T();
    const Array<float> F = find_fundamental_matrix(y0, y1);
    //std::cerr << fundamental_error(F, y0, y1) << std::endl;
    //std::cerr << F << std::endl;
    PpmImage bmp{ArrayShape{200, 200}, Rgb24::white()};
    highlight_features((dehomogenized_Nx2(y0)) * 180.f, bmp, 2, Rgb24::red());
    highlight_features((dehomogenized_Nx2(y1)) * 180.f, bmp, 2, Rgb24::blue());
    Array<float> p;
    Array<float> v;
    find_epiline(F, y0[1], p, v);
    assert_allclose(p, Array<float>{0.784089, 0.788539});
    assert_allclose(v, Array<float>{-0.703581, -0.070358});
    //std::cerr << "v " << v << std::endl;
    //std::cerr << (F, y0[0]) << std::endl;
    //std::cerr << (F.T(), y1[0]) << std::endl;
    //std::cerr << (y1[0], (F, y0[0]))() << std::endl;
    draw_epilines_from_F(F, bmp, Rgb24::green());
    draw_inverse_epilines_from_F(F, bmp, Rgb24::blue(), 72);
    bmp.draw_infinite_line(
        a2fi(p * 180.f),
        a2fi((p + v) * 180.f),
        0,
        Rgb24::black());
    bmp.save_to_file("TestOut/epiline.ppm");
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
//         for(double i = 1e-3; i > 1e-13; i/=1.5) {
//             std::cerr << numerical_differentiation([&](
//                     const Array<double>& kk){ return (rodrigues(kk), x); },
//                     k,
//                     double(i)) << std::endl;
//         }
//         std::cerr << rodrigues_jacobian_dk(k, x) << std::endl;
//     }
// }

void test_marginalized_map() {
    {
        MarginalizedMap<std::map<std::string, int>> mm;
        mm.active_["a"] = 5;
        size_t ncalls = 0;
        for(auto c : mm) {
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
        for(auto c : mm) {
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
        for(auto c : mm1) {
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
        for(auto c : mm1) {
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
        for(auto it = mm1.rbegin(); it != mm1.rend(); ++it) {
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
            for(auto it = mm.rbegin(); it != mm.rend(); ++it) {
                l.push_back(it->it_->second);
            }
            assert_true(l[0] == 9);
            assert_true(l[1] == 10);
            assert_true(l[2] == 7);
            assert_true(l[3] == 5);
        }
        {
            std::vector<int> l;
            for(auto c : mm) {
                l.push_back(c.it_->second);
            }
            assert_true(l[0] == 5);
            assert_true(l[1] == 7);
            assert_true(l[2] == 10);
            assert_true(l[3] == 9);
        }
        {
            std::vector<int> l;
            for(auto it = mm1.rbegin(); it != mm1.rend(); ++it) {
                l.push_back(it->it_->second);
            }
            assert_true(l[0] == 9);
            assert_true(l[1] == 10);
            assert_true(l[2] == 7);
            assert_true(l[3] == 5);
        }
        {
            std::vector<int> l;
            for(const auto& c : mm1) {
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
    return 0;
}
