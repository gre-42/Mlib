#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geometry/Cross.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Intersect_Lines.hpp>
#include <Mlib/Geometry/Intersection/Octree.hpp>
#include <Mlib/Geometry/Intersection/Point_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Mesh/Contour.hpp>
#include <Mlib/Geometry/Mesh/Lines_To_Rectangles.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Roundness_Estimator.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>

using namespace Mlib;

void test_cross() {
    FixedArray<float, 3> a{7, 3, 9};
    FixedArray<float, 3> b{5, 2, 8};
    assert_allclose(
        dot1d(cross(a), b).to_array(),
        cross(a, b).to_array());
    assert_allclose(
        dot1d(cross(a.to_array()), b.to_array()),
        cross(a, b).to_array());
    assert_allclose(
        dot1d(cross(a), b).to_array(),
        cross(a.to_array(), b.to_array()));
}

void test_contour() {
    OrderableFixedArray<float, 2> v00{FixedArray<float, 2>{0.f, 0.f}};
    OrderableFixedArray<float, 2> v10{FixedArray<float, 2>{1.f, 0.f}};
    OrderableFixedArray<float, 2> v11{FixedArray<float, 2>{1.f, 1.f}};
    OrderableFixedArray<float, 2> v01{FixedArray<float, 2>{0.f, 1.f}};
    OrderableFixedArray<float, 2> v02{FixedArray<float, 2>{0.f, 2.f}};
    std::vector<OrderableFixedArray<float, 2>> contour{
        v00,
        v10,
        v11,
        v01};
    std::list<FixedArray<OrderableFixedArray<float, 2>, 3>> triangles{
        {v00, v10, v01},
        {v00, v11, v01},
        {v01, v11, v02}};
    std::vector<std::list<FixedArray<OrderableFixedArray<float, 2>, 3>>> inner_triangles;
    delete_triangles_inside_contours(
        std::vector<std::vector<OrderableFixedArray<float, 2>>>{contour},
        triangles,
        inner_triangles);
    assert_true(triangles.size() == 0);
}

void test_contour2() {
    OrderableFixedArray<float, 2> v00{FixedArray<float, 2>{0.f, 0.f}};
    OrderableFixedArray<float, 2> v10{FixedArray<float, 2>{1.f, 0.f}};
    OrderableFixedArray<float, 2> v11{FixedArray<float, 2>{1.f, 1.f}};
    OrderableFixedArray<float, 2> v01{FixedArray<float, 2>{0.f, 1.f}};
    std::list<FixedArray<OrderableFixedArray<float, 2>, 3>> triangles{
        {v00, v01, v11},
        {v00, v11, v10}};
    std::vector<FixedArray<ColoredVertex, 3>> otriangles;
    for (const auto& t : triangles) {
        otriangles.push_back(FixedArray<ColoredVertex, 3>{
            ColoredVertex{.position = {t(0)(0), t(0)(1), 0.f}},
            ColoredVertex{.position = {t(1)(0), t(1)(1), 0.f}},
            ColoredVertex{.position = {t(2)(0), t(2)(1), 0.f}}});
    }
    for (const auto& t : triangles) {
        otriangles.push_back(FixedArray<ColoredVertex, 3>{
            ColoredVertex{.position = {t(0)(0) + 1.f, t(0)(1), 0.f}},
            ColoredVertex{.position = {t(1)(0) + 1.f, t(1)(1), 0.f}},
            ColoredVertex{.position = {t(2)(0) + 1.f, t(2)(1), 0.f}}});
    }
    std::swap(otriangles[1], otriangles[2]);
    find_contours(std::list(otriangles.begin(), otriangles.end()), ContourDetectionStrategy::EDGE_NEIGHBOR);
}

void test_invert_scaled_4x4() {
    auto R = tait_bryan_angles_2_matrix<float>({3.f, 2.f, 4.f});
    auto t = FixedArray<float, 3>{5, 1, 2};
    auto scale = 1.23f;
    auto m = assemble_homogeneous_4x4(R * scale, t);
    assert_allclose(dot2d(m, inverted_scaled_se3(m)).to_array(), identity_array<float>(4));
    assert_allclose(dot2d(inverted_scaled_se3(m), m).to_array(), identity_array<float>(4));
}

//void test_octree() {
//    Octree<float, 1, std::string> o;
//    o.insert(AxisAlignedBoundingBox<float, 1>{min_: 1, max_: 2}, "1-2");
//}

void test_intersect_lines() {
    FixedArray<FixedArray<float, 2>, 2> l0{FixedArray<float, 2>{-1, 2}, FixedArray<float, 2>{1, 2}};
    FixedArray<FixedArray<float, 2>, 2> l1{FixedArray<float, 2>{1.1, -3}, FixedArray<float, 2>{1.1, 4}};
    assert_allclose(intersect_lines(l0, l1, 0.f, 0.f).to_array(), Array<float>{1.1, 2});
    assert_allclose(intersect_lines(l0, l1, 0.1f, 0.f).to_array(), Array<float>{1.1, 2.05});
    assert_allclose(intersect_lines(l0, l1, 0.1f, 0.2f).to_array(), Array<float>{1, 2.05});
}

void test_lines_to_rectangles() {
    FixedArray<float, 2> p00;
    FixedArray<float, 2> p01;
    FixedArray<float, 2> p10;
    FixedArray<float, 2> p11;
    FixedArray<float, 2> aL{-1, 1};
    FixedArray<float, 2> aR{-2, -2};
    FixedArray<float, 2> b{0, 0};
    FixedArray<float, 2> c{1, 0};
    FixedArray<float, 2> dL{2, 1};
    FixedArray<float, 2> dR{2, -1};
    float width_aLb = 0.2;
    float width_aRb = 0.2;
    float width_bc = 0.2;
    float width_cdL = 0.2;
    float width_cdR = 0.2;
    lines_to_rectangles(
        p00,
        p01,
        p10,
        p11,
        aL,
        aR,
        b,
        c,
        dL,
        dR,
        width_aLb,
        width_aRb,
        width_bc,
        width_cdL,
        width_cdR);
    assert_allclose(p00.to_array(), Array<float>{0.0414214, 0.1});
    assert_allclose(p01.to_array(), Array<float>{0.0414214, -0.1});
    assert_allclose(p10.to_array(), Array<float>{0.958579, 0.1});
    assert_allclose(p11.to_array(), Array<float>{0.958579, -0.1});
}

FixedArray<float, 3> a2k(const FixedArray<float, 3>& angles) {
    return inverse_rodrigues(tait_bryan_angles_2_matrix(angles));
}

FixedArray<float, 3> k2a(const FixedArray<float, 3>& k) {
    return matrix_2_tait_bryan_angles(rodrigues(k));
}

void test_inverse_rodrigues() {
    {
        FixedArray<float, 3> k{1, 3, 1.2};
        k /= std::sqrt(sum(squared(k)));
        k *= 0.45;
        assert_allclose(k.to_array(), inverse_rodrigues(rodrigues(k)).to_array());
    }
    {
        FixedArray<float, 3> k{fixed_zeros<float, 3>()};
        assert_allclose(k.to_array(), inverse_rodrigues(rodrigues(k)).to_array());
    }
    {
        FixedArray<float, 3> a{-3.13672, -3.92299e-05, -3.14133};
        assert_allclose(
            tait_bryan_angles_2_matrix(a).to_array(),
            tait_bryan_angles_2_matrix(k2a(a2k(a))).to_array(),
            1e-4);
        assert_allclose(k2a(a2k(a)).to_array(), a.to_array(), 1e-4);
    }
}

void test_bvh() {
    Bvh<float, int, 3> bvh{{3, 4, 5}, 2};
    bvh.insert({{1, 2, 3}, {2, 3, 4}}, 42);
    bvh.insert({{1, 2, 3}, {2, 3, 4}}, 43);
    bvh.insert({{1, 20, 3}, {2, 23, 4}}, 44);
    bvh.insert({{1, 6, 3}, {2, 7, 4}}, 45);
    bvh.insert({{1, 3, 3}, {2, 4, 4}}, 46);    
    // bvh.visit({{0, 1, 2}, 4}, [](const int& data){
    //     std::cerr << category << " " << data << std::endl;
    // });
    // std::cerr << bvh << std::endl;
}

void test_bvh_performance() {
    {
        Bvh<float, int, 3> bvh{{3, 4, 5}, 2};
        bvh.insert({{1, 2, 3}, {2, 3, 4}}, 42);
        bvh.insert({{1, 2, 3}, {2, 3, 4}}, 43);
        bvh.insert({{1, 20, 3}, {2, 23, 4}}, 44);
        bvh.insert({{1, 6, 3}, {2, 7, 4}}, 45);
        bvh.insert({{1, 3, 3}, {2, 4, 4}}, 46);    
        bvh.visit({{0, 1, 2}, 4}, [](int data){
            std::cout << data << std::endl;
            return true;
        });
        std::cout << bvh << std::endl;
    }
    bool check_linear = false;
    for (size_t o = 0; o < 1 + (size_t)check_linear; ++o) {
        std::mt19937 gen(0); // Standard mersenne_twister_engine
        std::uniform_real_distribution<float> dis(-1, 1);
        Bvh<float, int, 3> bvh{{0.25f + 10.f * o, 0.2f + 10.f * o, 0.2f + 10.f * o}, 10};
        size_t nelems = 10 * 1000;
        bool compute_search_time = true; // is slow
        std::vector<float> nelements;
        std::vector<float> search_times;
        if (compute_search_time) {
            nelements.reserve(nelems);
            search_times.reserve(nelems);
        }
        for (size_t n = 0; n < nelems; ++n) {
            if (compute_search_time) {
                nelements.push_back(n);
                search_times.push_back(bvh.search_time());
            }
            FixedArray<float, 3> bmin{dis(gen), dis(gen), dis(gen)};
            bvh.insert({bmin, bmin + FixedArray<float, 3>{0.01, 0.02, 0.03}}, 42);
            // std::cout << "search time " << bvh.search_time() << std::endl;
        }
        if (compute_search_time) {
            std::ofstream ostr("perf" + std::to_string(o) + ".svg");
            Svg svg{ostr, 800, 600};
            svg.plot(nelements, search_times);
            svg.finish();
            ostr.flush();
            if (ostr.fail()) {
                throw std::runtime_error("Could not write perf file");
            }
        }
        bvh.print(std::cout, BvhPrintingOptions{
            .level = false,
            .aabb = false});
        if (false) {
            FixedArray<float, 3> center{0.012, 0.023, 0.045};
            bvh.insert({{0.01, 0.02, 0.03}, center}, 4321);
            bvh.print(std::cout, BvhPrintingOptions{
                .level = false,
                .aabb = false});
            bvh.visit({center, 0.01}, [](int data){
                std::cout << data << std::endl;
                return true;
            });
        }
    }
    if (false) {
        std::ofstream ostr("img.svg");
        Svg svg{ostr, 800, 600};
        svg.plot(std::vector<int>{0, 1, 2, 3, 4}, std::vector<int>{0*0, 1*1, 2*2, 3*3, 4*4});
        svg.finish();
        ostr.flush();
        if (ostr.fail()) {
            throw std::runtime_error("Could not write img file");
        }
    }
}

void test_roundness_estimator() {
    RoundnessEstimator rm;
    rm.add_direction({0, 1});
    assert_isclose(rm.roundness(), 0.f);
    rm.add_direction({0, -1});
    assert_isclose(rm.roundness(), 0.f);
    rm.add_direction({1, 0});
    assert_isclose(rm.roundness(), 4 * 0.2222222222f);
    rm.add_direction({-1, 0});
    assert_isclose(rm.roundness(), 4 * 0.25f);

    rm.add_direction({0, 1});
    assert_isclose(rm.roundness(), 4 * 0.24f);
    rm.add_direction({0, -1});
    assert_isclose(rm.roundness(), 4 * 0.2222222222f);
    rm.add_direction({1, 0});
    assert_isclose(rm.roundness(), 4 * 0.244898f);
    rm.add_direction({-1, 0});
    assert_isclose(rm.roundness(), 4 * 0.25f);

    rm.add_direction({-2, 2.1});
    assert_isclose(rm.roundness(), 0.737352f);
}

// void test_smoothen_edges() {
//     std::list<std::shared_ptr<TriangleList>> triangle_lists;
//     triangle_lists.push_back(std::make_shared<TriangleList>("", Material{}));
//     triangle_lists.back()->draw_triangle_wo_normals({0.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {0.5f, 1.f, 0.f});
//     triangle_lists.back()->draw_triangle_wo_normals({0.f, 0.f, 0.f}, {0.5f, -1.f, 0.4f}, {1.f, 0.f, 0.f});
//     TriangleList::smoothen_edges(triangle_lists, triangle_lists);
//     for (const auto& t : triangle_lists.back()->triangles_) {
//         std::cerr << std::endl;
//         for (const auto& v : t.flat_iterable()) {
//             std::cerr << v.position << std::endl;
//         }
//     }
// }

void test_distance_point_triangle() {
    assert_isclose(distance_point_to_triangle<float>({0, 0}, {0, 0}, {1, 0}, {1, 1}), 0.f);
    assert_isclose(distance_point_to_triangle<float>({0, -2}, {0, 0}, {1, 0}, {1, 1}), 2.f);
    assert_isclose(distance_point_to_triangle<float>({3.1, 0}, {0, 0}, {1, 0}, {1, 1}), 2.1f);

    assert_isclose(distance_point_to_triangle<float>({0.5, 0.2}, {0, 0}, {1, 0}, {1, 1}), 0.f);
}

int main(int argc, const char** argv) {
    enable_floating_point_exceptions();
    #ifdef _MSC_VER
    #pragma float_control(except, on)
    #endif

    test_cross();
    test_contour();
    test_contour2();
    // test_octree();
    test_invert_scaled_4x4();
    test_intersect_lines();
    test_lines_to_rectangles();
    test_inverse_rodrigues();
    test_bvh();
    // test_bvh_performance();
    test_roundness_estimator();
    // test_smoothen_edges();
    test_distance_point_triangle();
    return 0;
}
