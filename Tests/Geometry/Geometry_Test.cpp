#include <Mlib/Geometry/Cross.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Intersection/Intersect_Lines.hpp>
#include <Mlib/Geometry/Intersection/Octree.hpp>
#include <Mlib/Geometry/Mesh/Contour.hpp>
#include <Mlib/Geometry/Mesh/Lines_To_Rectangles.hpp>
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
    delete_triangles_outside_contour(
        contour,
        triangles);
    assert_true(triangles.size() == 0);
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
//    o.insert(BoundingBox<float, 1>{min_: 1, max_: 2}, "1-2");
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

int main(int argc, const char** argv) {
    test_cross();
    test_contour();
    // test_octree();
    test_invert_scaled_4x4();
    test_intersect_lines();
    test_lines_to_rectangles();
    return 0;
}
