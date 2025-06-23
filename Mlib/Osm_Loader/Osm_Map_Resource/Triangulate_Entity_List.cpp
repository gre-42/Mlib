#include "Triangulate_Entity_List.hpp"
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Contour_Intersections.hpp>
#include <Mlib/Geometry/Intersection/Intersect_Lines.hpp>
#include <Mlib/Geometry/Mesh/Indexed_Face_Set.hpp>
#include <Mlib/Geometry/Mesh/Plot.hpp>
#include <Mlib/Geometry/Mesh/Save_Obj.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Limits.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <vector>

using namespace Mlib;

void Mlib::plot_tris(const std::string& filename, const std::list<p2t::Triangle*>& tris) {
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

void Mlib::plot_tris(
    const std::string& filename,
    const std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& tris,
    const std::set<OrderableFixedArray<CompressedScenePos, 3>>& highlighted_points)
{
    std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> standard_triangles;
    std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> highlighted_triangles;
    for (const auto& t : tris) {
        bool is_highlighted = false;
        for (auto& v : t.flat_iterable()) {
            if (highlighted_points.contains(OrderableFixedArray(v.position))) {
                is_highlighted = true;
                break;
            }
        }
        if (is_highlighted) {
            highlighted_triangles.push_back(t);
        } else {
            standard_triangles.push_back(t);
        }
    }
    using L3 = std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>;
    using L4 = std::list<FixedArray<ColoredVertex<CompressedScenePos>, 4>>;
    save_obj(
        filename,
        IndexedFaceSet<float, CompressedScenePos, size_t>{std::vector{
            NamedInputPolygons<L3, L4>{"standard", "", standard_triangles, {}},
            NamedInputPolygons<L3, L4>{"highlighted", "", highlighted_triangles, {}} }},
        nullptr);  // material
}

void Mlib::plot_contours(const std::string& filename, const std::vector<std::vector<p2t::Point*>>& p2t_hole_contours, double scale) {
    std::list<std::list<FixedArray<CompressedScenePos, 2>>> contours;
    std::list<FixedArray<CompressedScenePos, 2>> highlighted_nodes;
    for (const auto& c : p2t_hole_contours) {
        auto& cs = contours.emplace_back();
        for (const auto& p : c) {
            cs.emplace_back(
                (CompressedScenePos)(p->x / scale),
                (CompressedScenePos)(-p->y / scale));
        }
        cs.emplace_back(
            (CompressedScenePos)(c.front()->x / scale),
            (CompressedScenePos)(-c.front()->y / scale));
    }
    {
        std::vector<std::vector<FixedArray<CompressedScenePos, 2>>> contours2;
        contours2.reserve(p2t_hole_contours.size());
        for (const auto& c : p2t_hole_contours) {
            auto& cs = contours2.emplace_back();
            cs.reserve(c.size());
            for (const auto& p : c) {
                cs.emplace_back(
                    (CompressedScenePos)(p->x / scale),
                    (CompressedScenePos)(-p->y / scale));
            }
        }
        visit_contour_intersections(
            contours2,
            [&highlighted_nodes](const FixedArray<ScenePos, 2>& intersection, size_t, size_t, ScenePos distance)
            {
                if (distance < (ScenePos)MIN_VERTEX_DISTANCE) {
                    highlighted_nodes.emplace_back(
                        intersection.casted<CompressedScenePos>());
                }
                return true;
            });
    }
    plot_mesh_svg(
        filename,
        600.,
        500.,
        {},
        {},
        contours,
        highlighted_nodes,
        (CompressedScenePos)0.1f); // line_width
}

void Mlib::check_contour(const std::vector<p2t::Point*>& contour) {
    if ((contour.front()->x == contour.back()->x) &&
        (contour.front()->y == contour.back()->y))
    {
        THROW_OR_ABORT("Triangulation boundary contour is closed");
    }
    if (compute_area_ccw(contour, 1.f) < 0.f) {
        THROW_OR_ABORT("Contour is not counterclockwise");
    }
}

// class VertexPointer {
// public:
//     p2t::Point* operator () (const p2t::Point& p) {
//         const OrderableFixedArray<double, 2>& f = reinterpret_cast<const OrderableFixedArray<double, 2>&>(p);
//         auto it = pts_.find(f);
//         if (it == pts_.end()) {
//             pts_.insert(f);
//             return const_cast<p2t::Point*>(reinterpret_cast<const p2t::Point*>(&*pts_.find(f)));
//         } else {
//             return const_cast<p2t::Point*>(reinterpret_cast<const p2t::Point*>(&*it));
//         }
//     }
//     bool empty() const {
//         return pts_.empty();
//     }
// private:
//     std::set<OrderableFixedArray<double, 2>> pts_;
// };
