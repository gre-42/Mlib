#include "Triangulate_3D.hpp"
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Distance_Point_Line.hpp>
#include <Mlib/Geometry/Intersection/Intersect_Lines.hpp>
#include <Mlib/Geometry/Mesh/Indexed_Point_Set.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
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

bool triangulate_point(
    const TransformationMatrix<float, 3>& point,
    const PointBvh& point_bvh,
    TriangleBvh& triangle_bvh,
    float boundary_radius,
    float z_thickness)
{
    TransformationMatrix<float, 3> projection = point.inverted();

    std::map<OrderableFixedArray<float, 2>, const FixedArray<float, 3>*> to3d;

    // Determine steiner points.
    BoundingSphere<float, 3> bounding_sphere{ point.t(), std::sqrt(2.f) * boundary_radius };
    std::list<FixedArray<float, 2>> steiner_points;
    if (!point_bvh.visit(
        bounding_sphere,
        [&](const Point3& point)
        {
            if (dot0d(point.R().column(2), projection.R()[2]) < 0) {
                return true;
            }
            FixedArray<float, 3> pt = projection.transform(point.t());
            if (std::abs(pt(2) > z_thickness)) {
                return true;
            }
            FixedArray<float, 2> p2{ pt(0), pt(1) };
            steiner_points.push_back(p2);
            if (!to3d.insert({OrderableFixedArray<float, 2>{p2}, &point.t()}).second) {
                std::cerr << "Detected duplicate Steiner point after projection" << std::endl;
                return false;
            }
            return true;
        }
    ))
    {
        return false;
    }
    IndexedPointSet indexed_points{steiner_points};

    // Add existing triangles.
    Array<int> old_triangles_{ ArrayShape{ 0 }};
    if (!triangle_bvh.visit(
        bounding_sphere,
        [&](const Triangle3& triangle)
        {
            if (dot0d(triangle.normal, projection.R()[2]) < 0) {
                return true;
            }
            FixedArray<FixedArray<float, 3>, 3> v{
                projection.transform(triangle.v(0)),
                projection.transform(triangle.v(1)),
                projection.transform(triangle.v(2))};
            for (size_t i = 0; i < 2; ++i) {
                if (std::abs(v(i)(2) > z_thickness)) {
                    return true;
                }
            }
            old_triangles_.append(indexed_points(v(0)(0), v(0)(1)));
            old_triangles_.append(indexed_points(v(1)(0), v(1)(1)));
            old_triangles_.append(indexed_points(v(2)(0), v(2)(1)));
            return true;
        }))
    {
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
        .numberoftriangles = (int)old_triangles_.length(),
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

    // Convert triangulation result to output format.
    for (int* i = out.trianglelist + 3 * in.numberoftriangles; i < out.trianglelist + 3 * out.numberoftriangles; i += 3) {
        FixedArray<OrderableFixedArray<float, 2>, 3> tri2{
            OrderableFixedArray<float, 2>{(float)out.pointlist[2 * (*i) + 0], (float)out.pointlist[2 * (*i) + 1]},
            OrderableFixedArray<float, 2>{(float)out.pointlist[2 * (*i) + 2], (float)out.pointlist[2 * (*i) + 3]},
            OrderableFixedArray<float, 2>{(float)out.pointlist[2 * (*i) + 4], (float)out.pointlist[2 * (*i) + 5]}};
        FixedArray<FixedArray<float, 3>, 3> tri3{
            *to3d.at(tri2(0)),
            *to3d.at(tri2(1)),
            *to3d.at(tri2(2))};
        triangle_bvh.insert(
            tri3,
            Triangle3{
                .v = tri3,
                .normal = triangle_normal(tri3)});
    }

    triangle::trifree(out.pointlist);
    triangle::trifree(out.pointmarkerlist);
    triangle::trifree(out.trianglelist);
    triangle::trifree(out.edgelist);
    triangle::trifree(out.edgemarkerlist);

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
