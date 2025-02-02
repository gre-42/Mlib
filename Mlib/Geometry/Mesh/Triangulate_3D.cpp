#include "Triangulate_3D.hpp"

#ifdef WITHOUT_TRIANGLE
#include <Mlib/Array/Array.hpp>

using namespace Mlib;

Array<FixedArray<float, 3, 3>> Mlib::triangulate_3d(
    const Array<TransformationMatrix<float, float, 3>>& points,
    float boundary_radius,
    float z_thickness,
    float cos_min_angle,
    float largest_cos_in_triangle,
    float triangle_search_eps)
{
    THROW_OR_ABORT("triangulate_3d: Compiled without Triangle library");
}
#else
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Distance_Point_Line.hpp>
#include <Mlib/Geometry/Intersection/Intersect_Lines.hpp>
#include <Mlib/Geometry/Intersection/Point_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Mesh/Indexed_Point_Set.hpp>
#include <Mlib/Geometry/Mesh/Plot.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Largest_Cosine.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Iterator/Reverse_Iterator.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Triangle/triangle.hpp>
#include <set>

using namespace Mlib;

struct Pose : public TransformationMatrix<float, float, 3> {
    const auto& primitive() const {
        return t;
    }
    const auto& payload() const {
        return *this;
    }
};

struct TriangleWithNormal {
    FixedArray<float, 3, 3> v;
    FixedArray<float, 3> normal;
};
using Point2 = FixedArray<float, 2>;
using Edge2 = FixedArray<float, 2, 2>;
using PoseBvh = GenericBvh<float, 3, PayloadContainer<std::vector<Pose>>>;
using TriangleBvh = Bvh<float, 3, TriangleWithNormal>;
using TrianglePointers = std::map<OrderableFixedArray<OrderableFixedArray<float, 3>, 3>, TriangleWithNormal*>;

class IndexedPointSet3D {
public:
    bool operator()(const Point2& p2, const FixedArray<float, 3>& p3, int& point_index) {
        point_index = indexed_points_(p2(0), p2(1));
        auto res = to3d_.insert({ point_index, &p3 });
        if (!res.second && !all(*res.first->second == p3)) {
            lerr() << "Detected duplicate Steiner point after projection";
            return false;
        }
        return true;
    }
    bool exists(const Point2& p2) {
        return indexed_points_.exists(p2(0), p2(1));
    }
    int next_index() {
        return indexed_points_.next_index();
    }
    const FixedArray<float, 3>& p3(int i) {
        return *to3d_.at(i);
    }
    std::vector<double>& positions() {
        return indexed_points_.positions();
    }
private:
    std::map<int, const FixedArray<float, 3>*> to3d_;
    IndexedPointSet indexed_points_;
};

void plot(Svg<double>& svg, const triangle::triangulateio& io) {
    if ((io.numberoftriangles == 0) && (io.numberofsegments == 0)) {
        return;
    }
    std::list<FixedArray<float, 3, 2>> triangles;
    for (int* i = io.trianglelist; i < io.trianglelist + 3 * io.numberoftriangles; i += 3) {
        triangles.push_back({
            Point2{(float)io.pointlist[2 * i[0]], (float)io.pointlist[2 * i[0] + 1]},
            Point2{(float)io.pointlist[2 * i[1]], (float)io.pointlist[2 * i[1] + 1]},
            Point2{(float)io.pointlist[2 * i[2]], (float)io.pointlist[2 * i[2] + 1]}});
    }
    std::list<std::list<Point2>> contours;
    for (int* i = io.segmentlist; i < io.segmentlist + 2 * io.numberofsegments; i += 2) {
        contours.push_back({
            Point2{(float)io.pointlist[2 * i[0]], (float)io.pointlist[2 * i[0] + 1]},
            Point2{(float)io.pointlist[2 * i[1]], (float)io.pointlist[2 * i[1] + 1]}});
    }
    plot_mesh(
        svg,
        triangles,
        {},
        contours,
        {},
        1.f);
}

void plot(const triangle::triangulateio& io, const std::string& filename, double width, double height) {
    std::ofstream ofstr{filename};
    Svg<double> svg{ ofstr, width, height };
    if (ofstr.fail()) {
        THROW_OR_ABORT("Could not open triangulation plot file \"" + filename + '"');
    }
    plot(svg, io);
    svg.finish();
    ofstr.flush();
    if (ofstr.fail()) {
        THROW_OR_ABORT("Could not write to file \"" + filename + '"');
    }
}

bool triangulate_point(
    size_t k,
    const TransformationMatrix<float, float, 3>& central_point,
    const PoseBvh& pose_bvh,
    TriangleBvh& triangle_bvh,
    TrianglePointers& triangle_ptrs,
    std::set<OrderableFixedArray<float, 3>>& triangulated_points,
    float boundary_radius,
    float z_thickness,
    float cos_min_angle,
    float largest_cos_in_triangle,
    float triangle_search_eps)
{
    triangulated_points.insert(OrderableFixedArray{ central_point.t });
    TransformationMatrix<float, float, 3> projection = central_point.inverted();

    // Determine steiner points.
    BoundingSphere<float, 3> bounding_sphere{ central_point.t, boundary_radius };
    auto bounding_box = AxisAlignedBoundingBox<float, 3>::from_center_and_radius(central_point.t, boundary_radius);
    auto bounding_box_plus_eps = AxisAlignedBoundingBox<float, 3>::from_center_and_radius(central_point.t, boundary_radius + triangle_search_eps);
    IndexedPointSet3D indexed_points;
    PointWithoutPayloadVectorBvh<float, 2> point2_bvh{ {0.05f, 0.05f}, 5 };
    if (!pose_bvh.visit(
        bounding_box,
        [&](const Pose& steiner_point)
        {
            if (!bounding_sphere.intersects(BoundingSphere<float, 3>{steiner_point.t, 0.f})) {
                return true;
            }
            if (dot0d(steiner_point.R.column(2), projection.R[2]) <= 0) {
                return true;
            }
            FixedArray<float, 3> pt = projection.transform(steiner_point.t);
            if (std::abs(pt(2)) > z_thickness) {
                return true;
            }
            Point2 pt2{ pt(0), pt(1) };
            int point_index;
            if (!indexed_points(pt2, steiner_point.t, point_index)) {
                return false;
            }
            point2_bvh.insert(PointWithoutPayload{ pt2 });
            return true;
        }
    ))
    {
        return false;
    }

    int steiner_point_index_end = indexed_points.next_index();

    // Add existing triangles.
    Array<int> segment_list{ ArrayShape{ 0 }};
    {
        Bvh<float, 2, Edge2> segment_bvh{{0.05f, 0.05f}, 5};
        if (!triangle_bvh.visit(
            bounding_box_plus_eps,
            [&](const TriangleWithNormal& triangle)
            {
                FixedArray<float, 3, 3> pr_tri3{
                    projection.transform(triangle.v[0]),
                    projection.transform(triangle.v[1]),
                    projection.transform(triangle.v[2])};
                FixedArray<float, 3, 2> pr_tri2{
                    Point2{ pr_tri3(0, 0), pr_tri3(0, 1) },
                    Point2{ pr_tri3(1, 0), pr_tri3(1, 1) },
                    Point2{ pr_tri3(2, 0), pr_tri3(2, 1) }};
                if (dot0d(triangle.normal, projection.R[2]) <= cos_min_angle) {
                    for (size_t i = 0; i < 3; ++i) {
                        if (indexed_points.exists(pr_tri2[i])) {
                            return false;
                        }
                    }
                    return true;
                }
                if (!point2_bvh.visit(AxisAlignedBoundingBox<float, 2>::from_points(pr_tri2), [&pr_tri2](const Point2& p2)
                    {
                        if (all(p2 == pr_tri2[0]) ||
                            all(p2 == pr_tri2[1]) ||
                            all(p2 == pr_tri2[2]))
                        {
                            return true;
                        }
                        if (point_is_in_triangle(p2, pr_tri2[0], pr_tri2[1], pr_tri2[2])) {
                            lerr() << "Detected point inside existing triangle";
                            return false;
                        }
                        return true;
                    }))
                {
                    return false;
                }
                for (size_t i = 0; i < 3; ++i) {
                    Edge2 edge2{
                        pr_tri2[i],
                        pr_tri2[(i + 1) % 3]};
                    auto aabb = AxisAlignedBoundingBox<float, 2>::from_points(edge2);
                    if (!segment_bvh.visit(aabb, [&edge2](const Edge2& other)
                        {
                            Point2 intersection = uninitialized;
                            if (intersect_lines(
                                intersection,
                                edge2,
                                other,
                                0.f,
                                0.f,
                                false,  // compute_center
                                true))  // check_bounds (returns false if any corners are identical)
                            {
                                lerr() << "Detected segment intersection";
                                return false;
                            }
                            return true;
                        }))
                    {
                        return false;
                    }
                    segment_bvh.insert(aabb, edge2);
                    int point_index;
                    if (!indexed_points(edge2[0], triangle.v[i], point_index)) {
                        return false;
                    }
                    segment_list.append(point_index);
                    if (!indexed_points(edge2[1], triangle.v[(i + 1) % 3], point_index)) {
                        return false;
                    }
                    segment_list.append(point_index);
                }
                return true;
            }))
        {
            return false;
        }
    }

    if (indexed_points.positions().size() / 2 < 3) {
        lerr() << "Less than 3 vertices for triangulation";
        return false;
    }

    // Triangulate.
    triangle::triangulateio in{
        .pointlist = indexed_points.positions().data(),
        .pointattributelist = nullptr,
        .pointmarkerlist = nullptr,
        .numberofpoints = (int)(indexed_points.positions().size() / 2),
        .numberofpointattributes = 0,
        
        .trianglelist = nullptr,
        .triangleattributelist = nullptr,
        .trianglearealist = nullptr,
        .neighborlist = nullptr,
        .numberoftriangles = 0,
        .numberofcorners = 0,
        .numberoftriangleattributes = 0,

        .segmentlist = segment_list.flat_begin(),
        .segmentmarkerlist = nullptr,
        .numberofsegments = (int)(segment_list.length() / 2),

        .holelist = nullptr,
        .numberofholes = 0,

        .regionlist = nullptr,
        .numberofregions = 0,

        // Out only
        .edgelist = nullptr,
        .edgemarkerlist = nullptr,
        .normlist = nullptr,
        .numberofedges = 0,
    };

    triangle::triangulateio out{
        .pointlist = nullptr,
        .pointattributelist = nullptr,
        .pointmarkerlist = nullptr,
        .numberofpoints = 0,
        .numberofpointattributes = 0,

        .trianglelist = nullptr,
        .triangleattributelist = nullptr,
        .trianglearealist = nullptr,
        .neighborlist = nullptr,
        .numberoftriangles = 0,
        .numberofcorners = 0,
        .numberoftriangleattributes = 0,

        .segmentlist = nullptr,
        .segmentmarkerlist = nullptr,
        .numberofsegments = 0,

        .holelist = nullptr,
        .numberofholes = 0,

        .regionlist = nullptr,
        .numberofregions = 0,

        // Out only
        .edgelist = nullptr,
        .edgemarkerlist = nullptr,
        .normlist = nullptr,
        .numberofedges = 0,
    };

    // lerr() << k;
    // if (k == 9513) {
    //     plot(in, "/tmp/plot_" + std::to_string(k) + "_in.svg", 100, 100);
    // }

    // Documentation:
    //   1. Read the comments in the triangle.hpp file.
    //   2. Run the triangle binary with "-h" and read the help text.
    // z: makes array indizes count from 0 (triangle counts from 1 otherwise)
    // -V -V -V: set printf debug level to 3
    // Q: quiet
    // https://www.cs.cmu.edu/~quake/triangle.quality.html
    // q5: Set minimum angle to 5 degrees.
    //     Not used because the angle is also applied to the input triangles,
    //     many of which have angles close to the prescribed quality.
    // p: enable the segment list
    // c: https://people.sc.fsu.edu/~jburkardt/data/triangle_files/triangle_files.html
    //    "We need the -pc because we didn't surround our initial area with segments.""
    triangle::triangulate("z e p c Q", &in, &out, nullptr);

    // if (k == 9513) {
    //     plot(out, "/tmp/plot_" + std::to_string(k) + "_out.svg", 100, 100);
    // }

    std::unique_ptr<double, decltype(&triangle::trifree)> unique_pointlist{ out.pointlist, triangle::trifree };
    std::unique_ptr<int, decltype(&triangle::trifree)> unique_pointmarkerlist{ out.pointmarkerlist, triangle::trifree };
    std::unique_ptr<int, decltype(&triangle::trifree)> unique_trianglelist{ out.trianglelist, triangle::trifree };
    std::unique_ptr<int, decltype(&triangle::trifree)> unique_edgelist{ out.edgelist, triangle::trifree };
    std::unique_ptr<int, decltype(&triangle::trifree)> unique_edgemarkerlist{ out.edgemarkerlist, triangle::trifree };

    if (out.numberofpoints != in.numberofpoints) {
        lerr() << "Out number of points differs from in number of points";
        return false;
    }
    if (out.numberoftriangles < in.numberoftriangles) {
        lerr() << "Triangles got removed";
        return false;
    }
    // for (int i = 0; i < 3 * in.numberoftriangles; ++i) {
    //     if (in.trianglelist[i] != out.trianglelist[i]) {
    //         lerr() << "Triangles got rearranged";
    //         return false;
    //     }
    // }
    
    // Convert triangulation result to output format.
    for (int j = 0; j < 3 * out.numberoftriangles; j += 3) {
        FixedArray<int, 3> i{
            out.trianglelist[j],
            out.trianglelist[j + 1],
            out.trianglelist[j + 2] };
        if (any(i >= steiner_point_index_end)) {
            continue;
        }
        FixedArray<float, 3, 2> tri2{
            Point2{ (float)out.pointlist[2 * i(0)], (float)out.pointlist[2 * i(0) + 1] },
            Point2{ (float)out.pointlist[2 * i(1)], (float)out.pointlist[2 * i(1) + 1] },
            Point2{ (float)out.pointlist[2 * i(2)], (float)out.pointlist[2 * i(2) + 1] }};
        {
            auto tlc = triangle_largest_cosine(tri2);
            if (std::isnan(tlc) || (tlc > largest_cos_in_triangle)) {
                continue;
            }
        }
        OrderableFixedArray<OrderableFixedArray<float, 3>, 3> otri3{
            OrderableFixedArray<float, 3>{ indexed_points.p3(i(0)) },
            OrderableFixedArray<float, 3>{ indexed_points.p3(i(1)) },
            OrderableFixedArray<float, 3>{ indexed_points.p3(i(2)) }};
        bool triangle_touches_future_point = false;
        for (const auto& p : otri3.flat_iterable()) {
            if (!triangulated_points.contains(p)) {
                triangle_touches_future_point = true;
                break;
            }
        }
        if (triangle_touches_future_point) {
            continue;
        }
        std::sort(otri3.flat_begin(), otri3.flat_end());
        auto pit = triangle_ptrs.find(otri3);
        if (pit != triangle_ptrs.end()) {
            continue;
        }
        FixedArray<float, 3, 3> tri3{
            indexed_points.p3(i(0)),
            indexed_points.p3(i(1)),
            indexed_points.p3(i(2))};
        const Mlib::AabbAndPayload<float, 3, TriangleWithNormal>& ttri3 = triangle_bvh.insert(
            AxisAlignedBoundingBox<float, 3>::from_points(tri3),
            TriangleWithNormal{
                .v = tri3,
                .normal = triangle_normal(tri3)});
        if (!triangle_ptrs.insert({ otri3, const_cast<TriangleWithNormal*>(&ttri3.payload()) }).second) {
            THROW_OR_ABORT("Triangulate internal error");
        }
    }

    return true;
}

Array<FixedArray<float, 3, 3>> Mlib::triangulate_3d(
    const Array<TransformationMatrix<float, float, 3>>& points,
    float boundary_radius,
    float z_thickness,
    float cos_min_angle,
    float largest_cos_in_triangle,
    float triangle_search_eps)
{
    TriangleBvh triangle_bvh{{0.1f, 0.1f, 0.1f}, 10};
    {
        PoseBvh pose_bvh{{0.1f, 0.1f, 0.1f}, 10};
        TrianglePointers triangle_ptrs;
        std::set<OrderableFixedArray<float, 3>> triangulated_points;
        for (const auto& p : points.flat_iterable()) {
            pose_bvh.insert(Pose{ p });
        }
        // Two passes, because in pass 2 the "triangulated_points" set is full.
        for (size_t i = 0; i < 2; ++i) {
            size_t k = 0;
            for (const TransformationMatrix<float, float, 3>& pt : points.flat_iterable()) {
                triangulate_point(
                    k,
                    pt,
                    pose_bvh,
                    triangle_bvh,
                    triangle_ptrs,
                    triangulated_points,
                    boundary_radius,
                    z_thickness,
                    cos_min_angle,
                    largest_cos_in_triangle,
                    triangle_search_eps);
                k++;
            }
        }
    }
    Array<FixedArray<float, 3, 3>> result{ ArrayShape{ 0 } };
    triangle_bvh.visit_all([&result](
        const AabbAndPayload<float, 3, TriangleWithNormal>& tri3)
    {
        result.append(tri3.payload().v);
        return true;
    });
    return result;
}

#endif
