#include <Mlib/Assert.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Geometry/Cameras/Frustum_Camera.hpp>
#include <Mlib/Geometry/Coordinates/Coordinate_Conversion.hpp>
#include <Mlib/Geometry/Coordinates/Cv_Look_At.hpp>
#include <Mlib/Geometry/Coordinates/Homogeneous.hpp>
#include <Mlib/Geometry/Cross.hpp>
#include <Mlib/Geometry/Fixed_Cross.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Bvh_Grid.hpp>
#include <Mlib/Geometry/Intersection/Caching_Bvh.hpp>
#include <Mlib/Geometry/Intersection/Distance/Distange_Polygon_Aabb.hpp>
#include <Mlib/Geometry/Intersection/Frustum3.hpp>
#include <Mlib/Geometry/Intersection/Intersect_Lines.hpp>
#include <Mlib/Geometry/Intersection/Intersectors/Ray_Segment_3D_For_Aabb.hpp>
#include <Mlib/Geometry/Intersection/Octree.hpp>
#include <Mlib/Geometry/Intersection/Point_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Intersection/Ray_Sphere_Intersection.hpp>
#include <Mlib/Geometry/Intersection/Welzl.hpp>
#include <Mlib/Geometry/Mesh/Contour.hpp>
#include <Mlib/Geometry/Mesh/Contour_Detection_Strategy.hpp>
#include <Mlib/Geometry/Mesh/Interpolated_Intermediate_Points_Creator.hpp>
#include <Mlib/Geometry/Mesh/Lines_To_Rectangles.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Mesh_Config.hpp>
#include <Mlib/Geometry/Mesh/Load/Load_Obj.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Height_Contours.hpp>
#include <Mlib/Geometry/Mesh/Point_And_Flags.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency_Impl.hpp>
#include <Mlib/Geometry/Mesh/Quad_Area.hpp>
#include <Mlib/Geometry/Mesh/Save_Obj.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Area.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Largest_Cosine.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Mesh/Triangulate_3D.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Geometry/Polygon_3D.hpp>
#include <Mlib/Geometry/Ray_Segment_3D.hpp>
#include <Mlib/Geometry/Roundness_Estimator.hpp>
#include <Mlib/Geometry/Shortest_Path_Multiple_Targets.hpp>
#include <Mlib/Geometry/Triangle_Is_Right_Handed.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Fixed_Test.hpp>
#include <Mlib/Math/Fixed_Test.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Math/Rodrigues.hpp>
#include <Mlib/Stats/Random_Arrays.hpp>
#include <poly2tri/poly2tri.h>

using namespace Mlib;

void test_special_tait_bryan_angles() {
    auto R = FixedArray<float, 3, 3>::init(
        (float)1.60828e-07, (float)0.207579, (float)0.978218,
        (float)0, (float)0.978218, (float)-0.207579,
        (float)-1, (float)3.33845e-08, (float)1.57325e-07);
    Quaternion<float> q{ R };
    assert_allclose(
        tait_bryan_angles_2_matrix(q.to_tait_bryan_angles()),
        R);
    assert_allclose(
        q.to_rotation_matrix(),
        R);
    assert_allclose(
        Quaternion<float>::from_tait_bryan_angles(q.to_tait_bryan_angles()).to_rotation_matrix(),
        R);
}

void test_inverse_tait_bryan_angles() {
    Array<float> kep = uniform_random_array<float>(ArrayShape{3}, 1);
    assert_allclose(kep, matrix_2_tait_bryan_angles(tait_bryan_angles_2_matrix(kep)));
    assert_allclose(
        Array<float>{kep(0), kep(1), 0},
        matrix_2_tait_bryan_angles(
            tait_bryan_angles_2_matrix(Array<float>{kep(0), kep(1), 0}),
            true));  // true == force_singular
}

void test_tait_bryan_angles_2_matrix() {
    {
        Array<float> r = tait_bryan_angles_2_matrix(Array<float>{0.f, 0.f, 0.f});
        assert_allclose(r, identity_array<float>(3));
        assert_allclose(outer(r, r), identity_array<float>(3));
    }

    {
        Array<float> r = tait_bryan_angles_2_matrix(Array<float>{1.f, 0.f, 0.f});
        assert_allclose(outer(r, r), identity_array<float>(3));
        assert_allclose(dot(r.T(), r), identity_array<float>(3));
    }

    {
        Array<float> r = tait_bryan_angles_2_matrix(Array<float>{0.1f, 0.2f, 0.3f});
        assert_allclose(outer(r, r), identity_array<float>(3));
        assert_allclose(dot(r.T(), r), identity_array<float>(3));
    }
}

void test_rodrigues_fixed() {
    Array<float> k = uniform_random_array<float>(ArrayShape{3}, 1);
    auto kf = FixedArray<float, 3>{k};
    FixedArray<float, 3, 3> rf = rodrigues1(kf);
    Array<float> r = rodrigues1(k);
    assert_allclose(r, rf.to_array());
}

void test_fixed_tait_bryan_angles_2_matrix() {
    Array<float> k = uniform_random_array<float>(ArrayShape{3}, 1);
    auto kf = FixedArray<float, 3>{k};
    auto rf = tait_bryan_angles_2_matrix(kf);
    auto r = tait_bryan_angles_2_matrix(k);
    assert_allclose(r, rf.to_array());

    assert_allclose(k, matrix_2_tait_bryan_angles(rf).to_array());
    kf(2) = 0;
    rf = tait_bryan_angles_2_matrix(kf);
    assert_allclose(kf.to_array(), matrix_2_tait_bryan_angles(rf, true).to_array());  // true=force_singular
}

void test_cross() {
    FixedArray<float, 3> a{7.f, 3.f, 9.f};
    FixedArray<float, 3> b{5.f, 2.f, 8.f};
    assert_allclose(
        dot1d(cross(a), b),
        cross(a, b));
    assert_allclose(
        dot1d(cross(a), b),
        cross(a, b));
    assert_allclose(
        dot1d(cross(a), b),
        cross(a, b));
}

void test_triangle_area() {
    FixedArray<float, 3> a{7.f, 3.f, 0.f};
    FixedArray<float, 3> b{5.f, 2.f, 0.f};
    FixedArray<float, 3> c{-6.f, -4.f, 0.f};
    assert_true(triangle_is_right_handed(a.row_range<0, 2>(), b.row_range<0, 2>(), c.row_range<0, 2>()));
    assert_isclose(triangle_area(a, b, c), 0.5f);
    assert_isclose(triangle_area(a.row_range<0, 2>(), b.row_range<0, 2>(), c.row_range<0, 2>()), 0.5f);
}

void test_quad_area() {
    FixedArray<float, 3> a{7.f, 3.f, 0.f};
    FixedArray<float, 3> b{5.f, 2.f, 0.f};
    FixedArray<float, 3> c{-6.f, -4.f, 0.f};
    FixedArray<float, 3> d{-3.f, -5.f, 0.f};
    assert_isclose(triangle_area(a, b, c), 0.5f);
    assert_isclose(triangle_area(a, c, d), 17.f);
    assert_isclose(quad_area(a, b, c, d), 17.5f);
    assert_isclose(quad_area(
        a.row_range<0, 2>(),
        b.row_range<0, 2>(),
        c.row_range<0, 2>(),
        d.row_range<0, 2>()), 17.5f);
}

void test_contour() {
    using P = CompressedScenePos;
    using V2 = FixedArray<P, 2>;
    FixedArray<P, 2> v00{V2{(P)0.f, (P)0.f}};
    FixedArray<P, 2> v10{V2{(P)1.f, (P)0.f}};
    FixedArray<P, 2> v11{V2{(P)1.f, (P)1.f}};
    FixedArray<P, 2> v01{V2{(P)0.f, (P)1.f}};
    FixedArray<P, 2> v02{V2{(P)0.f, (P)2.f}};
    std::vector<FixedArray<P, 2>> contour{
        v00,
        v10,
        v11,
        v01};
    std::list<FixedArray<P, 3, 2>> triangles{
        {v00, v10, v01},
        {v00, v11, v01},
        {v01, v11, v02}};
    std::vector<std::list<FixedArray<P, 3, 2>>> inner_triangles;
    extract_triangles_inside_contours(
        std::vector<std::vector<FixedArray<P, 2>>>{contour},
        triangles,
        inner_triangles,
        make_orderable_array);
    assert_true(triangles.size() == 1);
}

void test_contour2() {
    using P = CompressedScenePos;
    FixedArray<P, 2> v00{FixedArray<P, 2>{(P)0.f, (P)0.f}};
    FixedArray<P, 2> v10{FixedArray<P, 2>{(P)1.f, (P)0.f}};
    FixedArray<P, 2> v11{FixedArray<P, 2>{(P)1.f, (P)1.f}};
    FixedArray<P, 2> v01{FixedArray<P, 2>{(P)0.f, (P)1.f}};
    std::list<FixedArray<FixedArray<P, 2>, 3>> triangles{
        {v00, v01, v11},
        {v00, v11, v10}};
    std::vector<FixedArray<ColoredVertex<P>, 3>> otriangles;
    for (const auto& t : triangles) {
        otriangles.push_back(FixedArray<ColoredVertex<P>, 3>{
            ColoredVertex<P>{{t(0)(0), t(0)(1), (P)0.f}},
            ColoredVertex<P>{{t(1)(0), t(1)(1), (P)0.f}},
            ColoredVertex<P>{{t(2)(0), t(2)(1), (P)0.f}}});
    }
    for (const auto& t : triangles) {
        otriangles.push_back(FixedArray<ColoredVertex<P>, 3>{
            ColoredVertex<P>{{t(0)(0) + (P)1.f, t(0)(1), (P)0.f}},
            ColoredVertex<P>{{t(1)(0) + (P)1.f, t(1)(1), (P)0.f}},
            ColoredVertex<P>{{t(2)(0) + (P)1.f, t(2)(1), (P)0.f}}});
    }
    std::swap(otriangles[1], otriangles[2]);
    find_contours(std::list(otriangles.begin(), otriangles.end()), ContourDetectionStrategy::EDGE_NEIGHBOR);
}

void test_invert_scaled_4x4() {
    auto R = tait_bryan_angles_2_matrix(FixedArray<float, 3>{3.f, 2.f, 4.f});
    auto t = FixedArray<float, 3>{5.f, 1.f, 2.f};
    auto scale = 1.23f;
    auto m = assemble_homogeneous_4x4(R * scale, t);
    assert_allclose(dot2d(m, inverted_scaled_se3(m)), fixed_identity_array<float, 4>());
    assert_allclose(dot2d(inverted_scaled_se3(m), m), fixed_identity_array<float, 4>());
}

//void test_octree() {
//    Octree<float, 1, std::string> o;
//    o.insert(AxisAlignedBoundingBox<float, 1>{min_: 1, max_: 2}, "1-2");
//}

void test_intersect_lines() {
    FixedArray<float, 2, 2> l0{FixedArray<float, 2>{-1.f, 2.f}, FixedArray<float, 2>{1.f, 2.f}};
    FixedArray<float, 2, 2> l1{FixedArray<float, 2>{1.1f, -3.f}, FixedArray<float, 2>{1.1f, 4.f}};
    assert_allclose(intersect_lines(l0, l1, 0.f, 0.f), FixedArray<float, 2>{1.1f, 2.f});
    assert_allclose(intersect_lines(l0, l1, 0.1f, 0.f), FixedArray<float, 2>{1.1f, 2.05f});
    assert_allclose(intersect_lines(l0, l1, 0.1f, 0.2f), FixedArray<float, 2>{1.f, 2.05f});
}

void test_lines_to_rectangles() {
    using P = CompressedScenePos;
    FixedArray<CompressedScenePos, 2> p00 = uninitialized;
    FixedArray<CompressedScenePos, 2> p01 = uninitialized;
    FixedArray<CompressedScenePos, 2> p10 = uninitialized;
    FixedArray<CompressedScenePos, 2> p11 = uninitialized;
    FixedArray<CompressedScenePos, 2> aL{(P)-1., (P)1.};
    FixedArray<CompressedScenePos, 2> aR{(P)-2., (P)-2.};
    FixedArray<CompressedScenePos, 2> b{(P)0., (P)0.};
    FixedArray<CompressedScenePos, 2> c{(P)1., (P)0.};
    FixedArray<CompressedScenePos, 2> dL{(P)2., (P)1.};
    FixedArray<CompressedScenePos, 2> dR{(P)2., (P)-1.};
    CompressedScenePos width_aLb{ 0.2f };
    CompressedScenePos width_aRb{ 0.2f };
    CompressedScenePos width_bcL{ 0.2f };
    CompressedScenePos width_bcR{ 0.2f };
    CompressedScenePos width_cdL{ 0.2f };
    CompressedScenePos width_cdR{ 0.2f };
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
        width_bcL,
        width_bcR,
        width_cdL,
        width_cdR);
    assert_allclose(p00.casted<double>(), FixedArray<double, 2>{0.0414214f, 0.1f}, 1e-4);
    assert_allclose(p01.casted<double>(), FixedArray<double, 2>{0.0414214f, -0.1f}, 1e-4);
    assert_allclose(p10.casted<double>(), FixedArray<double, 2>{0.958579f, 0.1f}, 1e-4);
    assert_allclose(p11.casted<double>(), FixedArray<double, 2>{0.958579f, -0.1f}, 1e-4);
}

FixedArray<float, 3> a2k(const FixedArray<float, 3>& angles) {
    return inverse_rodrigues(tait_bryan_angles_2_matrix(angles));
}

FixedArray<float, 3> k2a(const FixedArray<float, 3>& k) {
    return matrix_2_tait_bryan_angles(rodrigues1(k));
}

void test_inverse_rodrigues() {
    {
        FixedArray<float, 3> k{1.f, 3.f, 1.2f};
        k /= std::sqrt(sum(squared(k)));
        k *= 0.45f;
        assert_allclose(k, inverse_rodrigues(rodrigues1(k)));
    }
    {
        FixedArray<float, 3> k{fixed_zeros<float, 3>()};
        assert_allclose(k, inverse_rodrigues(rodrigues1(k)));
    }
    {
        FixedArray<float, 3> a{-3.13672f, (float)-3.92299e-05, -3.14133f};
        assert_allclose(
            tait_bryan_angles_2_matrix(a),
            tait_bryan_angles_2_matrix(k2a(a2k(a))),
            (float)1e-4);
        assert_allclose(k2a(a2k(a)), a, (float)1e-4);
    }
}

void test_bvh() {
    using Payload = int;
    using BVH = Bvh<float, 3, Payload>;
    using AABB = AxisAlignedBoundingBox<float, 3>;
    BVH bvh{ {3.f, 4.f, 5.f}, 2 };
    bvh.insert(AABB::from_min_max({1.f, 2.f, 3.f}, {2.f, 3.f, 4.f}), 42);
    bvh.insert(AABB::from_min_max({1.f, 2.f, 3.f}, {2.f, 3.f, 4.f}), 43);
    bvh.insert(AABB::from_min_max({1.f, 20.f, 3.f}, {2.f, 23.f, 4.f}), 44);
    bvh.insert(AABB::from_min_max({1.f, 6.f, 3.f}, {2.f, 7.f, 4.f}), 45);
    bvh.insert(AABB::from_min_max({1.f, 3.f, 3.f}, {2.f, 4.f, 4.f}), 46);
    // bvh.visit({{0, 1, 2}, 4}, [](const int& data){
    //     lerr() << category << " " << data;
    // });
    // lerr() << bvh;
    {
        std::vector<std::pair<float, const int*>> result = bvh.min_distances<Payload>(3, FixedArray<float, 3>{1.5f, 2.5f, 3.5f}, 10.f, [](const auto& p) {return std::abs((float)p - 43.f); });
        assert_isequal(result.size(), (size_t)3);
        assert_isequal(*result[0].second, 43);
    }
    {
        std::vector<std::pair<float, const int*>> result = bvh.min_distances<Payload>(20, FixedArray<float, 3>{1.5f, 2.5f, 3.5f}, 100.f, [](const auto& p) {return std::abs((float)p - 43.f); });
        assert_isequal(result.size(), (size_t)5);
        assert_isequal(*result[0].second, 43);
        assert_isequal(*result[4].second, 46);
    }
    CachingBvh cvh{ bvh, Cache<BvhCacheElement<AABB, AabbAndPayload<float, 3, Payload>>>{ 100 } };
    cvh.visit(
        AABB::from_min_max({1.f, 2.f, 3.f}, {2.f, 3.f, 4.f}),
        AABB::from_min_max({1.f, 2.f, 3.f}, {2.f, 3.f, 4.f}),
        [](const auto& d) {
            linfo() << d;
            return true;
        });
    CachingAabbBvh<float, 3, Payload> cvh2{ bvh, 100 };
    cvh2.visit(
        AABB::from_min_max({1.f, 2.f, 3.f}, {2.f, 3.f, 4.f}),
        AABB::from_min_max({1.f, 2.f, 3.f}, {2.f, 3.f, 4.f}),
        [](const auto& d) {
            linfo() << d;
            return true;
        });
}

void test_bvh_performance() {
    using AABB = AxisAlignedBoundingBox<float, 3>;
    {
        Bvh<float, 3, int> bvh{{3.f, 4.f, 5.f}, 2};
        bvh.insert(AABB::from_min_max({1.f, 2.f, 3.f}, {2.f, 3.f, 4.f}), 42);
        bvh.insert(AABB::from_min_max({1.f, 2.f, 3.f}, {2.f, 3.f, 4.f}), 43);
        bvh.insert(AABB::from_min_max({1.f, 20.f, 3.f}, {2.f, 23.f, 4.f}), 44);
        bvh.insert(AABB::from_min_max({1.f, 6.f, 3.f}, {2.f, 7.f, 4.f}), 45);
        bvh.insert(AABB::from_min_max({1.f, 3.f, 3.f}, {2.f, 4.f, 4.f}), 46);
        bvh.visit(AABB::from_center_and_radius({0.f, 1.f, 2.f}, 4), [](int data){
            std::cout << data << std::endl;
            return true;
        });
        std::cout << bvh << std::endl;
    }
    bool check_linear = false;
    for (size_t o = 0; o < 1 + (size_t)check_linear; ++o) {
        std::mt19937 gen(0); // Standard mersenne_twister_engine
        std::uniform_real_distribution<float> dis(-1, 1);
        Bvh<float, 3, int> bvh{{0.25f + 10.f * (float)o, 0.2f + 10.f * (float)o, 0.2f + 10.f * (float)o}, 10};
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
                nelements.push_back((float)n);
                search_times.push_back(bvh.search_time(BvhDataRadiusType::NONZERO));
            }
            FixedArray<float, 3> bmin{dis(gen), dis(gen), dis(gen)};
            bvh.insert(AABB::from_min_max(bmin, bmin + FixedArray<float, 3>{0.01f, 0.02f, 0.03f}), 42);
            // std::cout << "search time " << bvh.search_time() << std::endl;
        }
        if (compute_search_time) {
            std::ofstream ostr("perf" + std::to_string(o) + ".svg");
            Svg svg{ostr, 800.f, 600.f};
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
    }
    if (false) {
        std::ofstream ostr("img.svg");
        Svg svg{ostr, 800.f, 600.f};
        svg.plot(std::vector<float>{0.f, 1.f, 2.f, 3.f, 4.f}, std::vector<float>{0.f*0.f, 1.f*1.f, 2.f*2.f, 3.f*3.f, 4.f*4.f});
        svg.finish();
        ostr.flush();
        if (ostr.fail()) {
            throw std::runtime_error("Could not write img file");
        }
    }
}

void test_interesection_grid() {
    using AABB = AxisAlignedBoundingBox<CompressedScenePos, 3>;
    using Grid = BvhGrid<CompressedScenePos, 3, int>;
    Grid grid{
        // BVH
        { (CompressedScenePos)0.2f, (CompressedScenePos)0.3f, (CompressedScenePos)0.4f },
        5,
        // Transition
        3,
        // Grid
        { 2u, 3u, 4u },
        { (CompressedScenePos)0.5f, (CompressedScenePos)0.6f, (CompressedScenePos)0.7f }
    };

    auto aabb = AABB::from_min_max(
        {(CompressedScenePos)1.f, (CompressedScenePos)2.f, (CompressedScenePos)3.f},
        {(CompressedScenePos)2.f, (CompressedScenePos)3.f, (CompressedScenePos)4.f});

    grid.root_bvh.insert(aabb, 42);
    grid.grid().visit(aabb, [](const auto& element){
        return true;
    });
}

void test_ray_segment_intersects_aabb() {
    using AABB = AxisAlignedBoundingBox<float, 3>;

    FixedArray<float, 3> start{ 1.f, 2.f, 3.f };
    FixedArray<float, 3> end{ 2.f, 3.f, 4.f };

    RaySegment3DForAabb<float, float> ray{ {start, end} };
    ray.intersects(AABB::from_min_max({1.f, 2.f, 3.f}, {2.f, 3.f, 4.f}));
}

void test_roundness_estimator() {
    RoundnessEstimator rm;
    rm.add_direction({0.f, 1.f});
    assert_isclose(rm.roundness(), 0.f);
    rm.add_direction({0.f, -1.f});
    assert_isclose(rm.roundness(), 0.f);
    rm.add_direction({1.f, 0.f});
    assert_isclose(rm.roundness(), 4 * 0.2222222222f);
    rm.add_direction({-1.f, 0.f});
    assert_isclose(rm.roundness(), 4 * 0.25f);

    rm.add_direction({0.f, 1.f});
    assert_isclose(rm.roundness(), 4 * 0.24f);
    rm.add_direction({0.f, -1.f});
    assert_isclose(rm.roundness(), 4 * 0.2222222222f);
    rm.add_direction({1.f, 0.f});
    assert_isclose(rm.roundness(), 4 * 0.244898f);
    rm.add_direction({-1.f, 0.f});
    assert_isclose(rm.roundness(), 4 * 0.25f);

    rm.add_direction({-2.f, 2.1f});
    assert_isclose(rm.roundness(), 0.737352f);
}

// void test_smoothen_edges() {
//     std::list<std::shared_ptr<TriangleList>> triangle_lists;
//     triangle_lists.push_back(std::make_shared<TriangleList>("", Material{}));
//     triangle_lists.back()->draw_triangle_wo_normals({0.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {0.5f, 1.f, 0.f});
//     triangle_lists.back()->draw_triangle_wo_normals({0.f, 0.f, 0.f}, {0.5f, -1.f, 0.4f}, {1.f, 0.f, 0.f});
//     TriangleList::smoothen_edges(triangle_lists, triangle_lists);
//     for (const auto& t : triangle_lists.back()->triangles_) {
//         lerr();
//         for (const auto& v : t.flat_iterable()) {
//             lerr() << v.position;
//         }
//     }
// }

void test_distance_point_triangle() {
    using V = FixedArray<float, 2>;
    assert_isclose(distance_point_to_triangle<float>({ 0.f, 0.f }, { V{0.f, 0.f}, V{1.f, 0.f}, V{1.f, 1.f} }), 0.f);
    assert_isclose(distance_point_to_triangle<float>({ 0.f, -2.f }, { V{0.f, 0.f}, V{1.f, 0.f}, V{1.f, 1.f} }), 2.f);
    assert_isclose(distance_point_to_triangle<float>({ 3.1f, 0.f }, { V{0.f, 0.f}, V{1.f, 0.f}, V{1.f, 1.f} }), 2.1f);

    assert_isclose(distance_point_to_triangle<float>({ 0.5f, 0.2f }, { V{0.f, 0.f}, V{1.f, 0.f}, V{1.f, 1.f} }), 0.f);
}

void test_triangulate_3d_1() {
    const Array<TransformationMatrix<float, float, 3>> points{
        TransformationMatrix<float, float, 3>{fixed_identity_array<float, 3>(), FixedArray<float, 3>{0.f, 0.f, 0.f}},
        TransformationMatrix<float, float, 3>{fixed_identity_array<float, 3>(), FixedArray<float, 3>{1.f, 0.f, 0.f}},
        TransformationMatrix<float, float, 3>{fixed_identity_array<float, 3>(), FixedArray<float, 3>{1.f, 1.f, 0.f}}
    };

    Array<FixedArray<float, 3, 3>> mesh = triangulate_3d(
        points,
        10.f,   // boundary_radius
        0.1f,   // z_thickness
        0.1f);  // cos_min_angle

    assert_allequal(
        Array<float>{ mesh },
        Array<float>{ Array<FixedArray<float, 3>>{
            Array<FixedArray<FixedArray<float, 3>, 3>>{
                FixedArray<FixedArray<float, 3>, 3>{
                    FixedArray<float, 3>{0.f, 0.f, 0.f},
                    FixedArray<float, 3>{1.f, 0.f, 0.f},
                    FixedArray<float, 3>{1.f, 1.f, 0.f}}}}});
}

TransformationMatrix<float, float, 3> generate_point_observation(const FixedArray<float, 3>& pos) {
    return opengl_to_cv_extrinsic_matrix(
        TransformationMatrix<float, float, 3>{
            gl_lookat_absolute(
                cv_to_opengl_coordinates({0.f, 2.f, -2.f}),
                cv_to_opengl_coordinates(pos),
                cv_to_opengl_coordinates({0.f, 1.f, 0.f})).value(),
            cv_to_opengl_coordinates(pos) });
}

void test_triangulate_3d_2() {
    const Array<TransformationMatrix<float, float, 3>> points{
        generate_point_observation({0.f, 0.f, -1.f}),
        generate_point_observation({0.f, 0.f, 0.f}),
        generate_point_observation({1.f, 0.f, 0.f}),
        generate_point_observation({1.f, 1.f, 0.f})
    };

    Array<FixedArray<float, 3, 3>> mesh = triangulate_3d(
        points,
        1.f,    // boundary_radius
        10.f,   // z_thickness
        -10.f); // cos_min_angle

    assert_allequal(
        Array<float>{ mesh },
        Array<float>{ Array<FixedArray<float, 3>>{
            Array<FixedArray<FixedArray<float, 3>, 3>>{
                FixedArray<FixedArray<float, 3>, 3>{
                    FixedArray<float, 3>{1.f, 0.f, 0.f},
                    FixedArray<float, 3>{0.f, 0.f, 0.f},
                    FixedArray<float, 3>{0.f, 0.f, -1.f}},
                FixedArray<FixedArray<float, 3>, 3>{
                    FixedArray<float, 3>{1.f, 0.f, 0.f},
                    FixedArray<float, 3>{1.f, 1.f, 0.f},
                    FixedArray<float, 3>{0.f, 0.f, 0.f}}}}});
}

void test_smallest_angle_in_triangle() {
    assert_isclose(
        triangle_largest_cosine(FixedArray<float, 3, 2>{
            FixedArray<float, 2>{-1.f, -1.f},
            FixedArray<float, 2>{1.f, -1.f},
            FixedArray<float, 2>{0.f, 2.f}}),
        0.8f);
}

void test_rotate_intrinsic_matrix() {
    TransformationMatrix<float, float, 2> intrinsic_matrix{FixedArray<float, 3, 3>::init(
        100.f, 0.f, 51.f,
        0.f, 200.f, 107.f,
        0.f, 0.f, 1.f)};
    FixedArray<size_t, 2> sensor_size{ 100u, 200u };
    assert_allclose(
        rotated_intrinsic_matrix(intrinsic_matrix, sensor_size, 1).affine(),
        FixedArray<float, 3, 3>::init(
            200.f, 0.f, 107.f,
            0.f, 100.f, 49.f,
            0.f, 0.f, 1.f));
    assert_allclose(
        rotated_intrinsic_matrix(intrinsic_matrix, sensor_size, -1).affine(),
        FixedArray<float, 3, 3>::init(
            200.f, 0.f, 93.f,
            0.f, 100.f, 51.f,
            0.f, 0.f, 1.f));
    assert_allclose(
        rotated_intrinsic_matrix(intrinsic_matrix, sensor_size, -1).affine(),
        rotated_intrinsic_matrix(intrinsic_matrix, sensor_size, 3).affine());
    assert_allclose(
        intrinsic_matrix.affine(),
        rotated_intrinsic_matrix(intrinsic_matrix, sensor_size, 0).affine());
    assert_allclose(
        intrinsic_matrix.affine(),
        rotated_intrinsic_matrix(intrinsic_matrix, sensor_size, 4).affine());
}

enum class TestPointFlags {
    A = 1 << 0,
    B = 1 << 1,
    C = 1 << 2
};

static inline TestPointFlags operator | (TestPointFlags a, TestPointFlags b) {
    return (TestPointFlags)((int)a | (int)b);
}

static inline TestPointFlags& operator |= (TestPointFlags& a, TestPointFlags b) {
    return (TestPointFlags&)((int&)a |= (int)b);
}

static inline std::ostream& operator << (std::ostream& ostr, TestPointFlags f) {
    return (ostr << (int)f);
}

void test_subdivide_points_and_adjacency() {
    using TPoint = PointAndFlags<FixedArray<float, 2>, TestPointFlags>;
    PointsAndAdjacency<TPoint> pa;
    pa.points = { TPoint{{0.1f, 0.2f}, TestPointFlags::A }, TPoint{{0.78f, 0.56f}, TestPointFlags::B} };
    pa.adjacency = SparseArrayCcs<float>(2, 2);
    pa.adjacency(0, 1) = 0.4f;
    pa.adjacency(1, 0) = 0.4f;
    linfo() << '\n' << pa.adjacency;
    auto interpolate = interpolate_default<TPoint>;
    InterpolatedIntermediatePointsCreator<TPoint, decltype(interpolate)> iipc(0.1f, interpolate);
    pa.subdivide(
        [&](size_t r, size_t c, const float& distance) {
            return iipc(pa.points.at(r), pa.points.at(c), distance);
        },
        SubdivisionType::SYMMETRIC);
    linfo() << '\n' << pa.adjacency;
    for (const auto& p : pa.points) {
        linfo() << p;
    }
}

void test_combine_points_and_adjacency() {
    using TPoint = PointAndFlags<FixedArray<float, 2>, TestPointFlags>;
    PointsAndAdjacency<TPoint> pa;
    pa.points = {
        TPoint{{0.1f, 0.2f}, TestPointFlags::A },
        TPoint{{0.78f, 0.56f}, TestPointFlags::B},
        TPoint{{0.79f, 0.56f}, TestPointFlags::C} };
    pa.adjacency = SparseArrayCcs<float>(3, 3);
    pa.adjacency(0, 1) = 0.4f;
    pa.adjacency(1, 0) = 0.4f;
    pa.adjacency(1, 2) = 0.01f;
    linfo() << '\n' << pa.adjacency;
    pa.merge_neighbors(1e-1f, 5e-1f, [](auto& a, const auto& b) { a |= b; });
    linfo() << '\n' << pa.adjacency;
    for (const auto& p : pa.points) {
        linfo() << p;
    }
}

void test_welzl_triangle() {
    FixedArray<float, 2> a{1.f, 2.f};
    FixedArray<float, 2> b{1.f, 2.f};
    FixedArray<float, 2> c{4.f, 3.f};
    auto rng = welzl_rng();
    std::vector<const FixedArray<float, 2>*> a_b_c{&a, &b, &c};
    std::vector<const FixedArray<float, 2>*> a_c{&a, &c};
    assert_allequal(welzl_from_vector<float, 2>(a_b_c, rng).center, (a + c) / 2.f);
    assert_allequal(welzl_from_vector<float, 2>(a_c, rng).center, (a + c) / 2.f);
}

void test_welzl_tetrahedron() {
    FixedArray<float, 3> a{1.f, 2.f, 0.f};
    FixedArray<float, 3> b{1.f, 2.f, 0.f};
    FixedArray<float, 3> c{4.f, 3.f, 0.f};
    FixedArray<float, 3> d{4.f, 3.f, 0.f};
    auto rng = welzl_rng();
    std::vector<const FixedArray<float, 3>*> a_b_c_d{&a, &b, &c, &d};
    std::vector<const FixedArray<float, 3>*> a_c{&a, &c};
    assert_allequal(welzl_from_vector<float, 3>(a_b_c_d, rng).center, (a + c) / 2.f);
    assert_allequal(welzl_from_vector<float, 3>(a_c, rng).center, (a + c) / 2.f);
}

void test_shortest_path() {
    PointsAndAdjacency<FixedArray<double, 2>> points_and_adjacency;
    points_and_adjacency.adjacency = SparseArrayCcs<double>{ArrayShape{4, 4}};
    points_and_adjacency.points = UUVector<FixedArray<double, 2>>{
        FixedArray<double, 2>{0., 0.},
        FixedArray<double, 2>{1., 0.},
        FixedArray<double, 2>{1., 1.},
        FixedArray<double, 2>{0., 1.}};
    points_and_adjacency.adjacency(0, 0) = 0.;
    points_and_adjacency.adjacency(1, 1) = 0.;
    points_and_adjacency.adjacency(2, 2) = 0.;
    points_and_adjacency.adjacency(3, 3) = 0.;
    points_and_adjacency.adjacency(0, 1) = std::sqrt(sum(squared(points_and_adjacency.points[0] - points_and_adjacency.points[1])));
    points_and_adjacency.adjacency(1, 0) = std::sqrt(sum(squared(points_and_adjacency.points[1] - points_and_adjacency.points[0])));
    points_and_adjacency.adjacency(1, 2) = std::sqrt(sum(squared(points_and_adjacency.points[1] - points_and_adjacency.points[2])));
    points_and_adjacency.adjacency(2, 1) = std::sqrt(sum(squared(points_and_adjacency.points[2] - points_and_adjacency.points[1])));
    points_and_adjacency.adjacency(2, 3) = std::sqrt(sum(squared(points_and_adjacency.points[2] - points_and_adjacency.points[3])));
    points_and_adjacency.adjacency(3, 2) = std::sqrt(sum(squared(points_and_adjacency.points[3] - points_and_adjacency.points[2])));
    std::vector<size_t> targets{{ 0, 2 }};
    std::vector<size_t> predecessors;
    std::vector<double> total_distances;
    shortest_path_multiple_targets(
        points_and_adjacency,
        targets,
        predecessors,
        total_distances);
    assert_allequal(Array<size_t>{predecessors}, Array<size_t>{SIZE_MAX, 2, SIZE_MAX, 2});
}

void test_frustum3() {
    FrustumCameraConfig cfg{
        .near_plane = 2.f,
        .far_plane = 100.f,
        .left = -2.f,
        .right = 2.f,
        .bottom = -2.f,
        .top = 2.f};
    FrustumCamera camera{cfg, FrustumCamera::Postprocessing::DISABLED};
    auto frustum3 = Frustum3<float>::from_projection_matrix(camera.projection_matrix());
    frustum3.normalize();
    assert_allclose(frustum3.near_plane().normal, FixedArray<float, 3>{0.f, 0.f, -1.f});
    assert_isclose(frustum3.near_plane().intercept, -2.f);
    assert_allclose(frustum3.far_plane().normal, FixedArray<float, 3>{0.f, 0.f, 1.f});
    assert_isclose(frustum3.far_plane().intercept, 100.f, (float)1e-3);
    assert_isequal(frustum3.intersects(AxisAlignedBoundingBox<float, 3>::from_min_max(
        {-1.f, -1.f, 1.f},
        {1.f, 1.f, 2.f})), false);
    assert_isequal(frustum3.intersects(AxisAlignedBoundingBox<float, 3>::from_min_max(
        {-1.f, -1.f, -2.1f},
        {1.f, 1.f, -3.1f})), true);
}

void test_ray_sphere_intersection() {
    FixedArray<double, 3> R{ 15., 0., 0. };
    FixedArray<double, 3> v{ 1., 0., 0. };
    FixedArray<double, 3> C{ 10., 0., 0. };
    double r = 7;
    double lambda = NAN;
    assert_isequal(ray_intersects_sphere(R, v, C, squared(r), &lambda, (double*)nullptr), true);
    assert_isclose(lambda, 2.);
}

void test_distance_polygon_aabb() {
    using P = CompressedScenePos;
    auto aabb = AxisAlignedBoundingBox<ScenePos, 3>::from_min_max(
        {1.f, 2.f, 3.f},
        {2.f, 3.f, 4.f});
    FixedArray<P, 3> a{(P)1., (P)2., (P)10.};
    FixedArray<P, 3> b{(P)2., (P)2., (P)10.};
    FixedArray<P, 3> c{(P)2., (P)3., (P)10.};
    FixedArray<P, 3> d{(P)1., (P)3., (P)10.};
    Polygon3D<P, 4> poly{{a, b, c, d}};
    auto rng = welzl_rng();
    CollisionPolygonSphere<P, 4> cps{
        .bounding_sphere = poly.bounding_sphere(rng).casted<P>(),
        .polygon = poly.polygon().casted<SceneDir, P>(),
        .physics_material = PhysicsMaterial::NONE,
        .corners = poly.vertices().casted<P>()};
    ClosestPoint<SceneDir, ScenePos> cp;
    distance_polygon_aabb<4>(cps, aabb, cp);
    linfo() << cp.closest_point0 << " - " << cp.closest_point1 << " - " << cp.normal << " - " << cp.distance;
}

void test_plane_shift() {
    NormalRandomNumberGenerator<SceneDir> drng{ 42 };
    NormalRandomNumberGenerator<ScenePos> prng{ 43, 0.f, 1'000.f };
    for (size_t i = 0; i < 1000; ++i) {
        PlaneNd<SceneDir, CompressedScenePos, 3> plane{
            FixedArray<SceneDir, 3>{ drng(), drng(), drng() },
            (CompressedScenePos)prng() };
        FixedArray<CompressedScenePos, 3> p{
            (CompressedScenePos)prng(),
            (CompressedScenePos)prng(),
            (CompressedScenePos)prng()};
        auto plane2 = plane + p;
        auto plane3 = plane2 - p;
        if (plane3 != plane) {
            THROW_OR_ABORT("Plane roundtrip error");
        }
    }
}

void plot_tris(const std::string& filename, const std::vector<p2t::Triangle*>& tris) {
    std::list<FixedArray<ColoredVertex<double>, 3>> triangles;
    for (const auto& t : tris) {
        triangles.push_back(FixedArray<ColoredVertex<double>, 3>{
            ColoredVertex<double>{{t->GetPoint(0)->x, t->GetPoint(0)->y, 0.}, Colors::WHITE, {0.f, 0.f}, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f}},
            ColoredVertex<double>{{t->GetPoint(1)->x, t->GetPoint(1)->y, 0.}, Colors::WHITE, {0.f, 0.f}, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f}},
            ColoredVertex<double>{{t->GetPoint(2)->x, t->GetPoint(2)->y, 0.}, Colors::WHITE, {0.f, 0.f}, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f}}
        });
    }
    save_obj(
        filename,
        IndexedFaceSet<float, double, size_t>{ triangles },
        nullptr);  // material
}

void test_touching_holes() {
    std::list<p2t::Point> points;
    std::vector<p2t::Point*> polyline{
        &points.emplace_back(0, 0),
        &points.emplace_back(1, 0),
        &points.emplace_back(0.5, 1),
    };
    auto p2t_Point_02_02 = p2t::Point(0.2, 0.2);
    std::vector<p2t::Point*> hole0 {
        &points.emplace_back(0.1, 0.1),
        &points.emplace_back(0.2, 0.1),
        &p2t_Point_02_02,
    };
    std::vector<p2t::Point*> hole1 {
        &p2t_Point_02_02,
        &points.emplace_back(0.3, 0.2),
        &points.emplace_back(0.3, 0.3),
    };
    p2t::CDT cdt{ polyline };
    cdt.AddHole(hole0);
    cdt.AddHole(hole1);
    cdt.Triangulate();
    // auto result = cdt.GetMap();
    auto result = cdt.GetTriangles();
    // plot_tris("/tmp/tris_test.obj", result);
}

void test_height_contours() {
    auto triangles = load_obj<CompressedScenePos>(
        "Data/box.obj",
        LoadMeshConfig<CompressedScenePos>{
            .blend_mode = BlendMode::OFF,
            .cull_faces_default = true,
            .cull_faces_alpha = true,
            .occluded_pass = ExternalRenderPassType::NONE,
            .occluder_pass = ExternalRenderPassType::NONE,
            .magnifying_interpolation_mode = InterpolationMode::NEAREST,
            .aggregate_mode = AggregateMode::NONE,
            .transformation_mode = TransformationMode::ALL,
            .period_world = INFINITY,
            .apply_static_lighting = false,
            .laplace_ao_strength = 0.f,
            .dynamically_lighted = false,
            .physics_material = PhysicsMaterial::ATTR_VISIBLE,
            .werror = true});
    height_contours(triangles, (CompressedScenePos)0.1f);
}

int main(int argc, const char** argv) {
    enable_floating_point_exceptions();

    try {
        test_touching_holes();

        test_special_tait_bryan_angles();
        test_tait_bryan_angles_2_matrix();
        test_inverse_tait_bryan_angles();
        test_rodrigues_fixed();
        test_fixed_tait_bryan_angles_2_matrix();

        test_cross();
        test_triangle_area();
        test_quad_area();
        test_contour();
        test_contour2();
        // test_octree();
        test_invert_scaled_4x4();
        test_intersect_lines();
        test_lines_to_rectangles();
        test_inverse_rodrigues();
        test_bvh();
        // test_bvh_performance();
        test_interesection_grid();
        test_ray_segment_intersects_aabb();
        test_roundness_estimator();
        // test_smoothen_edges();
        test_distance_point_triangle();
    #ifndef WITHOUT_TRIANGLE
        test_triangulate_3d_1();
        test_triangulate_3d_2();
    #endif
        test_smallest_angle_in_triangle();
        test_rotate_intrinsic_matrix();
        // test_subdivide_points_and_adjacency();
        // test_combine_points_and_adjacency();
        test_welzl_triangle();
        test_welzl_tetrahedron();
        test_shortest_path();
        test_frustum3();
        test_ray_sphere_intersection();
        test_distance_polygon_aabb();
        test_plane_shift();
        test_height_contours();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
