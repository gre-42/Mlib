#include "Triangulate_3D.hpp"
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Distance_Point_Line.hpp>
#include <Mlib/Geometry/Intersection/Intersect_Lines.hpp>
#include <Mlib/Geometry/Mesh/Indexed_Point_Set.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Reverse_Iterator.hpp>
#include <Triangle/triangle.hpp>

using namespace Mlib;

typedef TransformationMatrix<float, 3> Point3;
struct Triangle3 {
    FixedArray<FixedArray<float, 3>, 3> v;
    FixedArray<float, 3> normal;
};
typedef Bvh<float, Point3, 3> PointBvh;
typedef Bvh<float, Triangle3, 3> TriangleBvh;

class IndexedPointSet3D {
public:
    bool operator()(const FixedArray<float, 2>& p2, const FixedArray<float, 3>& p3, int& point_index) {
        point_index = indexed_points_(p2(0), p2(1));
        auto res = to3d_.insert({ point_index, &p3 });
        if (!res.second && !all(*res.first->second == p3)) {
            std::cerr << "Detected duplicate Steiner point after projection" << std::endl;
            return false;
        }
        return true;
    }
    bool exists(const FixedArray<float, 2>& p2) {
        return indexed_points_.exists(p2(0), p2(1));
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

bool triangulate_point(
    const TransformationMatrix<float, 3>& central_point,
    const PointBvh& point_bvh,
    TriangleBvh& triangle_bvh,
    float boundary_radius,
    float z_thickness)
{
    TransformationMatrix<float, 3> projection = central_point.inverted();

    // Determine steiner points.
    BoundingSphere<float, 3> bounding_sphere{ central_point.t(), std::sqrt(2.f) * boundary_radius };
    IndexedPointSet3D indexed_points;
    if (!point_bvh.visit(
        bounding_sphere,
        [&](const Point3& steiner_point)
        {
            if (dot0d(steiner_point.R().column(2), projection.R()[2]) <= 0) {
                return true;
            }
            FixedArray<float, 3> pt = projection.transform(steiner_point.t());
            if (std::abs(pt(2)) > z_thickness) {
                return true;
            }
            int point_index;
            if (!indexed_points(FixedArray<float, 2>{ pt(0), pt(1) }, steiner_point.t(), point_index)) {
                return false;
            }
            return true;
        }
    ))
    {
        return false;
    }

    // Add existing triangles.
    Array<int> old_triangles_{ ArrayShape{ 0 }};
    if (!triangle_bvh.visit(
        bounding_sphere,
        [&](const Triangle3& triangle)
        {
            FixedArray<FixedArray<float, 3>, 3> v{
                projection.transform(triangle.v(0)),
                projection.transform(triangle.v(1)),
                projection.transform(triangle.v(2))};
            if (dot0d(triangle.normal, projection.R()[2]) <= 0) {
                for (size_t i = 0; i < 3; ++i) {
                    if (indexed_points.exists(FixedArray<float, 2>{v(i)(0), v(i)(1)})) {
                        return false;
                    }
                }
                return true;
            }
            for (size_t i = 0; i < 3; ++i) {
                int point_index;
                if (!indexed_points(FixedArray<float, 2>{v(i)(0), v(i)(1)}, triangle.v(i), point_index)) {
                    return false;
                }
                old_triangles_.append(point_index);
            }
            return true;
        }))
    {
        return false;
    }

    if (indexed_points.positions().size() / 2 < 3) {
        std::cerr << "Less than 3 vertices for triangulation" << std::endl;
        return false;
    }

    // Triangulate.
    triangle::triangulateio in{
        .pointlist = indexed_points.positions().data(),
        .pointattributelist = nullptr,
        .pointmarkerlist = nullptr,
        .numberofpoints = (int)(indexed_points.positions().size() / 2),
        .numberofpointattributes = 0,
        .trianglelist = old_triangles_.flat_begin(),
        .triangleattributelist = nullptr,
        .trianglearealist = nullptr,
        .neighborlist = nullptr,
        .numberoftriangles = (int)(old_triangles_.length() / 3),
        .numberofcorners = 0,
        .numberoftriangleattributes = 0,
        .segmentlist = nullptr,
        .segmentmarkerlist = nullptr,
        .numberofsegments = 0,
        .holelist = nullptr,
        .numberofholes = 0,
        .regionlist = nullptr,
        .numberofregions = 0,
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
        .edgelist = nullptr,
        .edgemarkerlist = nullptr,
        .normlist = nullptr,
        .numberofedges = 0,
    };

    // z: makes array indizes count from 0 (triangle counts from 1 otherwise)
    // -V -V -V: set printf debug level to 3
    // Q: quiet
    triangle::triangulate("z e Q", &in, &out, nullptr);

    std::unique_ptr<double, decltype(&triangle::trifree)> unique_pointlist{ out.pointlist, triangle::trifree };
    std::unique_ptr<int, decltype(&triangle::trifree)> unique_pointmarkerlist{ out.pointmarkerlist, triangle::trifree };
    std::unique_ptr<int, decltype(&triangle::trifree)> unique_trianglelist{ out.trianglelist, triangle::trifree };
    std::unique_ptr<int, decltype(&triangle::trifree)> unique_edgelist{ out.edgelist, triangle::trifree };
    std::unique_ptr<int, decltype(&triangle::trifree)> unique_edgemarkerlist{ out.edgemarkerlist, triangle::trifree };

    if (out.numberofpoints != in.numberofpoints) {
        std::cerr << "Out number of points differs from in number of points" << std::endl;
        return false;
    }
    // Convert triangulation result to output format.
    for (int* i = out.trianglelist + 3 * in.numberoftriangles; i < out.trianglelist + 3 * out.numberoftriangles; i += 3) {
        FixedArray<FixedArray<float, 3>, 3> tri3{
            indexed_points.p3(i[0]),
            indexed_points.p3(i[1]),
            indexed_points.p3(i[2])};
        bool good = true;
        for (size_t i = 0; i < 3; ++i) {
            if (!bounding_sphere.intersects(BoundingSphere<float, 3>{tri3(i), 0.f})) {
                good = false;
                break;
            }
        }
        if (!good) {
            continue;
        }
        triangle_bvh.insert(
            tri3,
            Triangle3{
                .v = tri3,
                .normal = triangle_normal(tri3, TriangleNormalErrorBehavior::WARN)});
    }

    return true;
}

Array<FixedArray<FixedArray<float, 3>, 3>> Mlib::triangulate_3d(
    const Array<TransformationMatrix<float, 3>>& points,
    float boundary_radius,
    float z_thickness)
{
    TriangleBvh triangle_bvh{{0.1f, 0.1f, 0.1f}, 10};
    PointBvh point_bvh{{0.1f, 0.1f, 0.1f}, 10};
    for (const auto& p : points.flat_iterable()) {
        point_bvh.insert(p.t(), p);
    }
    for (const TransformationMatrix<float, 3>& pt : points.flat_iterable()) {
        triangulate_point(
            pt,
            point_bvh,
            triangle_bvh,
            boundary_radius,
            z_thickness);
    }
    Array<FixedArray<FixedArray<float, 3>, 3>> result{ ArrayShape{ 0 } };
    triangle_bvh.visit_all([&result](const std::pair<AxisAlignedBoundingBox<float, 3>, Triangle3>& tri3){
        result.append(tri3.second.v);
        return true;
    });
    return result;
}
