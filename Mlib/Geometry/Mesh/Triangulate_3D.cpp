#include "Triangulate_3D.hpp"
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Distance_Point_Line.hpp>
#include <Mlib/Geometry/Intersection/Intersect_Lines.hpp>
#include <Mlib/Geometry/Mesh/Indexed_Point_Set.hpp>
#include <Mlib/Geometry/Mesh/Plot.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Largest_Cosine.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Reverse_Iterator.hpp>
#include <Triangle/triangle.hpp>
#include <set>

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

void plot(Svg<float>& svg, const triangle::triangulateio& io) {
    if (io.numberoftriangles == 0) {
        return;
    }
    std::list<FixedArray<FixedArray<float, 2>, 3>> triangles;
    for (int* i = io.trianglelist; i < io.trianglelist + 3 * io.numberoftriangles; i += 3) {
        triangles.push_back({
            FixedArray<float, 2>{(float)io.pointlist[2 * i[0]], (float)io.pointlist[2 * i[0] + 1]},
            FixedArray<float, 2>{(float)io.pointlist[2 * i[1]], (float)io.pointlist[2 * i[1] + 1]},
            FixedArray<float, 2>{(float)io.pointlist[2 * i[2]], (float)io.pointlist[2 * i[2] + 1]}});
    }
    plot_mesh(
        svg,
        triangles,
        {},
        {},
        1.f);
}

void plot(const triangle::triangulateio& io, const std::string& filename, float width, float height) {
    std::ofstream ofstr{filename};
    Svg<float> svg{ofstr, width, height};
    if (ofstr.fail()) {
        throw std::runtime_error("Could not open file \"" + filename + '"');
    }
    plot(svg, io);
    svg.finish();
    ofstr.flush();
    if (ofstr.fail()) {
        throw std::runtime_error("Could not write to file \"" + filename + '"');
    }
}

class OrderedTriangle {
public:
    OrderedTriangle(const FixedArray<int, 3>& p)
    : p_{p},
      s_{p}
    {
        std::sort(s_.flat_begin(), s_.flat_end());
    }
    int operator [] (size_t id) const {
        return p_(id);
    }
    bool operator < (const OrderedTriangle& other) const {
        return s_ < other.s_;
    }
private:
    FixedArray<int, 3> p_;
    OrderableFixedArray<int, 3> s_;
};

std::set<OrderedTriangle> get_sorted_triangles(const triangle::triangulateio& io) {
    std::set<OrderedTriangle> result;
    for (int i = 0; i < 3 * io.numberoftriangles; i += 3) {
        FixedArray<int, 3> tri{
            io.trianglelist[i],
            io.trianglelist[i + 1],
            io.trianglelist[i + 2] };
        if (!result.insert(OrderedTriangle{ tri }).second) {
            throw std::runtime_error("Duplicate triangle");
        }
    }
    return result;
}

bool triangulate_point(
    const TransformationMatrix<float, 3>& central_point,
    const PointBvh& point_bvh,
    TriangleBvh& triangle_bvh,
    float boundary_radius,
    float z_thickness,
    float cos_min_angle,
    float largest_cos_in_triangle,
    float triangle_search_eps)
{
    TransformationMatrix<float, 3> projection = central_point.inverted();

    // Determine steiner points.
    BoundingSphere<float, 3> bounding_sphere{ central_point.t(), boundary_radius };
    AxisAlignedBoundingBox<float, 3> bounding_box{ central_point.t(), boundary_radius };
    AxisAlignedBoundingBox<float, 3> bounding_box_plus_eps{ central_point.t(), boundary_radius + triangle_search_eps };
    IndexedPointSet3D indexed_points;
    if (!point_bvh.visit(
        bounding_box,
        [&](const Point3& steiner_point)
        {
            if (!bounding_sphere.intersects(BoundingSphere<float, 3>{steiner_point.t(), 0.f})) {
                return true;
            }
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

    int steiner_point_index_end = indexed_points.next_index();

    // Add existing triangles.
    Array<int> old_triangles{ ArrayShape{ 0 }};
    if (!triangle_bvh.visit(
        bounding_box_plus_eps,
        [&](const Triangle3& triangle)
        {
            FixedArray<FixedArray<float, 3>, 3> v{
                projection.transform(triangle.v(0)),
                projection.transform(triangle.v(1)),
                projection.transform(triangle.v(2))};
            if (dot0d(triangle.normal, projection.R()[2]) <= cos_min_angle) {
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
                old_triangles.append(point_index);
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
        .trianglelist = old_triangles.flat_begin(),
        .triangleattributelist = nullptr,
        .trianglearealist = nullptr,
        .neighborlist = nullptr,
        .numberoftriangles = (int)(old_triangles.length() / 3),
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
    // https://www.cs.cmu.edu/~quake/triangle.quality.html
    // q5: Set minimum angle to 5 degrees.
    //     Not used because the angle is also applied to the input triangles,
    //     many of which have angles close to the prescribed quality.
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
    if (out.numberoftriangles < in.numberoftriangles) {
        std::cerr << "Triangles got removed (0)" << std::endl;
        return false;
    }
    // for (int i = 0; i < 3 * in.numberoftriangles; ++i) {
    //     if (in.trianglelist[i] != out.trianglelist[i]) {
    //         std::cerr << "Triangles got rearranged" << std::endl;
    //         return false;
    //     }
    // }
    std::set<OrderedTriangle> sorted_old_triangles = get_sorted_triangles(in);
    std::set<OrderedTriangle> sorted_new_triangles = get_sorted_triangles(out);
    
    for (const auto& ot : sorted_old_triangles) {
        if (!sorted_new_triangles.contains(ot)) {
            std::cerr << "Triangles got removed (1)" << std::endl;
            return false;
        }
    }
    // Convert triangulation result to output format.
    for (const auto& i : sorted_new_triangles) {
        if ((i[0] >= steiner_point_index_end) ||
            (i[1] >= steiner_point_index_end) ||
            (i[2] >= steiner_point_index_end))
        {
            continue;
        }
        if (sorted_old_triangles.contains(i)) {
            continue;
        }
        FixedArray<FixedArray<float, 2>, 3> tri2{
            FixedArray<float, 2>{ (float)out.pointlist[2 * i[0]], (float)out.pointlist[2 * i[0] + 1] },
            FixedArray<float, 2>{ (float)out.pointlist[2 * i[1]], (float)out.pointlist[2 * i[1] + 1] },
            FixedArray<float, 2>{ (float)out.pointlist[2 * i[2]], (float)out.pointlist[2 * i[2] + 1] }};
        if (triangle_largest_cosine(tri2) > largest_cos_in_triangle) {
            continue;
        }
        FixedArray<FixedArray<float, 3>, 3> tri3{
            indexed_points.p3(i[0]),
            indexed_points.p3(i[1]),
            indexed_points.p3(i[2])};
        triangle_bvh.insert(
            tri3,
            Triangle3{
                .v = tri3,
                .normal = triangle_normal(tri3)});
    }

    // static int k = 0;
    // plot(in, "/tmp/plot_in_" + std::to_string(k) + ".svg", 100, 100);
    // plot(out, "/tmp/plot_out_" + std::to_string(k) + ".svg", 100, 100);
    // assert_true(k != 10);
    // ++k;

    return true;
}

Array<FixedArray<FixedArray<float, 3>, 3>> Mlib::triangulate_3d(
    const Array<TransformationMatrix<float, 3>>& points,
    float boundary_radius,
    float z_thickness,
    float cos_min_angle,
    float largest_cos_in_triangle,
    float triangle_search_eps)
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
            z_thickness,
            cos_min_angle,
            largest_cos_in_triangle,
            triangle_search_eps);
    }
    Array<FixedArray<FixedArray<float, 3>, 3>> result{ ArrayShape{ 0 } };
    triangle_bvh.visit_all([&result](const std::pair<AxisAlignedBoundingBox<float, 3>, Triangle3>& tri3){
        result.append(tri3.second.v);
        return true;
    });
    return result;
}
