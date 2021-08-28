#include "Triangulate_3D.hpp"
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Distance_Point_Line.hpp>
#include <Mlib/Geometry/Intersection/Intersect_Lines.hpp>
#include <Mlib/Geometry/Mesh/P2t_Point_Set.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Reverse_Iterator.hpp>
#include <poly2tri/poly2tri.h>
#include <set>

using namespace Mlib;

typedef TransformationMatrix<float, 3> Point3;
struct Edge3 {
    FixedArray<float, 3> a;
    FixedArray<float, 3> b;
    FixedArray<float, 3> normal;
    bool deleted;
};
typedef Bvh<float, Point3, 3> PointBvh;
typedef Bvh<float, Edge3, 3> EdgeBvh;

bool triangulate_point(
    Array<FixedArray<FixedArray<float, 3>, 3>>& mesh,
    const TransformationMatrix<float, 3>& point,
    const PointBvh& point_bvh,
    const EdgeBvh& edge_bvh,
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
    P2tPointSet p2t_points{steiner_points};

    // Define complete edges and artificial points.
    std::set<std::pair<OrderableFixedArray<float, 2>, OrderableFixedArray<float, 2>>> artificial_edges;
    std::set<OrderableFixedArray<float, 2>> artificial_points;

    // Compute original bounding rectangle.
    Array<FixedArray<float, 2>> original_boundary{
        FixedArray<float, 2>{-boundary_radius, -boundary_radius},
        FixedArray<float, 2>{+boundary_radius, -boundary_radius},
        FixedArray<float, 2>{+boundary_radius, +boundary_radius},
        FixedArray<float, 2>{-boundary_radius, +boundary_radius}};

    // Define connections between points (artificial ones like intersections,
    // complete edges, or corners of the original bounding rectangle).
    std::map<OrderableFixedArray<float, 2>, FixedArray<float, 2>> successors;
    auto connect = [&successors](const FixedArray<float, 2>& a, const FixedArray<float, 2>& b){
        if (!successors.insert({OrderableFixedArray<float, 2>(a), b}).second) {
            std::cerr << "Could not insert into edge map" << std::endl;
            return false;
        }
        return true;
    };

    // Add intersection points to boundaries and connect edges.
    Array<std::map<float, FixedArray<float, 2>>> boundary_intersections{ ArrayShape{ original_boundary.length() }};
    std::map<FixedArray<float, 2>*, std::pair<size_t, float>> edge_end_2_intersection_position;
    if (!edge_bvh.visit(
        bounding_sphere,
        [&](const Edge3& edge)
        {
            if (edge.deleted) {
                return true;
            }
            if (dot0d(edge.normal, projection.R()[2]) < 0) {
                return true;
            }
            FixedArray<float, 3> edge0 = projection.transform(edge.a);
            FixedArray<float, 3> edge1 = projection.transform(edge.b);
            if (std::abs(edge0(2) > z_thickness)) {
                return true;
            }
            if (std::abs(edge1(2) > z_thickness)) {
                return true;
            }
            FixedArray<float, 2> ep0{edge0(0), edge0(1)};
            FixedArray<float, 2> ep1{edge1(0), edge1(1)};
            {
                auto it = to3d.insert({OrderableFixedArray{ep0}, &edge.a});
                if (it.second && (!all(*it.first->second == edge.b))) {
                    std::cerr << "Detected duplicate vertex after projection (a)" << std::endl;
                    return false;
                }
            }
            {
                auto it = to3d.insert({OrderableFixedArray{ep1}, &edge.b});
                if (it.second && (!all(*it.first->second == edge.a))) {
                    std::cerr << "Detected duplicate vertex after projection (b)" << std::endl;
                    return false;
                }
            }
            for (size_t boundary_id = 0; boundary_id < original_boundary.length(); ++boundary_id) {
                const FixedArray<float, 2>& b0 = original_boundary(boundary_id);
                const FixedArray<float, 2>& b1 = original_boundary((boundary_id + 1) % original_boundary.length());
                FixedArray<float, 2> intersection;
                if (intersect_lines(
                    intersection,
                    FixedArray<FixedArray<float, 2>, 2>{ ep0, ep1 },
                    FixedArray<FixedArray<float, 2>, 2>{ b0, b1 },
                    0.f,            // width0
                    0.f,            // width1
                    false,          // compute_center
                    true))          // check_bounds
                {
                    bool ep0_in = (transform_to_line_coordinates(ep0, b0, b1)(1) < 0);
                    bool ep1_in = (transform_to_line_coordinates(ep1, b0, b1)(1) < 0);
                    if (ep0_in == ep1_in) {
                        std::cerr << "Can not determine in-ness" << std::endl;
                        return false;
                    }
                    FixedArray<float, 2> lc = transform_to_line_coordinates(intersection, b0, b1);
                    if (!boundary_intersections(boundary_id).insert({lc(0), intersection}).second)
                    {
                        std::cerr << "Duplicate boundary intersection" << std::endl;
                        return false;
                    }

                    if (ep1_in) {
                        connect(intersection, ep1);
                        if (!artificial_edges.insert(std::make_pair(OrderableFixedArray{intersection}, OrderableFixedArray{ep1})).second) {
                            std::cerr << "Could not insert artificial ingoing edge" << std::endl;
                            return false;
                        }
                    } else {
                        connect(ep0, intersection);
                        if (!artificial_edges.insert(std::make_pair(OrderableFixedArray{ep0}, OrderableFixedArray{intersection})).second) {
                            std::cerr << "Could not insert artificial outgoing edge" << std::endl;
                            return false;
                        }
                    }
                    if (!artificial_points.insert(OrderableFixedArray{intersection}).second) {
                        std::cerr << "Could not insert artificial point" << std::endl;
                        return false;
                    }
                } else {
                    if (!connect(ep0, ep1)) {
                        return false;
                    }
                }
            }
            return true;
        }))
    {
        return false;
    }
    
    // Connect intersection points with boundary corners.
    // Do not overwrite existing connections.
    for (size_t boundary_id0 = 0; boundary_id0 < original_boundary.length(); ++boundary_id0) {
        size_t boundary_id1 = (boundary_id0 + 1) % original_boundary.length();
        if (!boundary_intersections(boundary_id0).insert({
            0.f,
            original_boundary(boundary_id0)}).second)
        {
            std::cerr << "Could not insert start boundary intersection" << std::endl;
            return false;
        }
        if (!boundary_intersections(boundary_id0).insert({
            1.f,
            original_boundary(boundary_id1)}).second)
        {
            std::cerr << "Could not insert end boundary intersection" << std::endl;
            return false;
        }
        for (auto itb = boundary_intersections(boundary_id0).begin(); ; ) {
            auto ita = itb++;
            if (itb == boundary_intersections(boundary_id0).end()) {
                break;
            }
            if (!connect(ita->second, itb->second)) {
                return false;
            }
            // No error check, ignore existing points.
            artificial_points.insert(OrderableFixedArray{ita->second});
            artificial_points.insert(OrderableFixedArray{itb->second});
        }
    }
    
    std::vector<p2t::Point*> final_bounding_contour;
    // Compute contour from connected points.
    {
        if (successors.empty()) {
            std::cerr << "Successors array is empty" << std::endl;
            return false;
        }
        std::set<OrderableFixedArray<float, 2>> visited_points;
        FixedArray<float, 2> current_point = successors.begin()->first;
        while (!visited_points.contains(OrderableFixedArray{current_point})) {
            visited_points.insert(OrderableFixedArray{current_point});
            final_bounding_contour.push_back(p2t_points(current_point(0), current_point(1)));
            auto it = successors.find(OrderableFixedArray{current_point});
            if (it == successors.end()) {
                std::cerr << "Could not find successor" << std::endl;
                return false;
            }
            current_point = it->second;
        }
        if (!all(current_point == successors.begin()->first)) {
            std::cerr << "Contour not closed" << std::endl;
            return false;
        }
    }
    
    if (final_bounding_contour.size() < 3) {
        std::cerr << "Final bounding contour has length < 3" << std::endl;
        return false;
    }
    // Triangulate.
    p2t::CDT cdt{final_bounding_contour};
    for (const auto& p : p2t_points.remaining_steiner_points()) {
        cdt.AddPoint(p);
    }
    cdt.Triangulate();

    // Convert triangulation result to output format.
    mesh.resize(ArrayShape{ 0 });
    for (const auto& t : cdt.GetTriangles()) {
        FixedArray<OrderableFixedArray<float, 2>, 3> tri2{
            OrderableFixedArray<float, 2>{(float)t->GetPoint(0)->x, (float)t->GetPoint(0)->y},
            OrderableFixedArray<float, 2>{(float)t->GetPoint(1)->x, (float)t->GetPoint(1)->y},
            OrderableFixedArray<float, 2>{(float)t->GetPoint(2)->x, (float)t->GetPoint(2)->y}};
        bool valid = true;
        for (size_t i = 0; i < 3; ++i) {
            if (artificial_points.contains(tri2(i))) {
                valid = false;
                break;
            }
            if (artificial_edges.contains(std::make_pair(tri2(i), tri2((i + 1) % 3)))) {
                valid = false;
                break;
            }
        }
        if (!valid) {
            continue;
        }
        mesh.append(FixedArray<FixedArray<float, 3>, 3>{
            *to3d.at(tri2(0)),
            *to3d.at(tri2(1)),
            *to3d.at(tri2(2))});
    }

    return true;
}

Array<FixedArray<FixedArray<float, 3>, 3>> Mlib::triangulate_3d(
    const Array<TransformationMatrix<float, 3>>& points,
    float boundary_radius,
    float z_thickness)
{
    Array<FixedArray<FixedArray<float, 3>, 3>> result;
    EdgeBvh edge_bvh{{0.1f, 0.1f, 0.1f}, 10};
    PointBvh point_bvh{{0.1f, 0.1f, 0.1f}, 10};
    for (const auto& p : points.flat_iterable()) {
        point_bvh.insert(p.t(), p);
    }
    for (const TransformationMatrix<float, 3>& pt : points.flat_iterable()) {
        triangulate_point(
            result,
            pt,
            point_bvh,
            edge_bvh,
            boundary_radius,
            z_thickness);
    }
    return result;
}
