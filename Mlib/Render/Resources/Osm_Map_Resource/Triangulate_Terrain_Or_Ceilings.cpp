#include "Triangulate_Terrain_Or_Ceilings.hpp"
#include <Mlib/Geometry/Mesh/Contour.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Bounding_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <poly2tri/poly2tri.h>

using namespace Mlib;

namespace Mlib {

class PTri {
public:
    p2t::Point* operator () (size_t i) const {
        return v->GetPoint((int)i);
    }
private:
    p2t::Triangle* v;
};

}

void Mlib::triangulate_terrain_or_ceilings(
    TriangleList& tl_terrain,
    TriangleList* tl_terrain_visuals,
    const std::list<std::list<FixedArray<ColoredVertex, 3>>>& tl_insert,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const std::vector<FixedArray<float, 2>>& bounding_contour,
    const std::list<FixedArray<ColoredVertex, 3>>& hole_triangles,
    float scale,
    float uv_scale,
    float z,
    const FixedArray<float, 3>& color)
{
    p2t::Point p00{bounding_info.boundary_min(0) - bounding_info.border_width, bounding_info.boundary_min(1) - bounding_info.border_width};
    p2t::Point p01{bounding_info.boundary_min(0) - bounding_info.border_width, bounding_info.boundary_max(1) + bounding_info.border_width};
    p2t::Point p10{bounding_info.boundary_max(0) + bounding_info.border_width, bounding_info.boundary_min(1) - bounding_info.border_width};
    p2t::Point p11{bounding_info.boundary_max(0) + bounding_info.border_width, bounding_info.boundary_max(1) + bounding_info.border_width};
    std::vector<p2t::Point> p2t_bounding_nodes;
    p2t_bounding_nodes.reserve(bounding_contour.size());
    std::vector<p2t::Point*> p2t_bounding_contour;
    p2t_bounding_contour.reserve(bounding_contour.size());
    for (const auto& p : bounding_contour) {
        p2t_bounding_nodes.push_back(p2t::Point{p(0), p(1)});
        p2t_bounding_contour.push_back(&p2t_bounding_nodes.back());
    }
    auto final_bounding_contour = bounding_contour.empty()
        ? std::vector<p2t::Point*>{&p00, &p01, &p11, &p10}
        : p2t_bounding_contour;
    p2t::CDT cdt{final_bounding_contour};
    //PolygonDrawer pd;
    //pd.draw_line(p00, p01, 10);
    //pd.draw_line(p01, p11, 10);
    //pd.draw_line(p11, p10, 10);
    //pd.draw_line(p10, p00, 10);
    //p2t::CDT cdt{pd.points_};
    if (false) {
        std::vector<p2t::Point> p2t_nodes;
        p2t_nodes.reserve(3 * hole_triangles.size());
        for (const auto& t : hole_triangles) {
            if (std::find(p2t_nodes.begin(), p2t_nodes.end(), p2t::Point{t(0).position(0), t(0).position(1)}) != p2t_nodes.end()) continue;
            if (std::find(p2t_nodes.begin(), p2t_nodes.end(), p2t::Point{t(1).position(0), t(1).position(1)}) != p2t_nodes.end()) continue;
            if (std::find(p2t_nodes.begin(), p2t_nodes.end(), p2t::Point{t(2).position(0), t(2).position(1)}) != p2t_nodes.end()) continue;
            p2t_nodes.push_back(p2t::Point{t(0).position(0), t(0).position(1)});
            p2t_nodes.push_back(p2t::Point{t(1).position(0), t(1).position(1)});
            p2t_nodes.push_back(p2t::Point{t(2).position(0), t(2).position(1)});
            cdt.AddHole(std::vector<p2t::Point*>{
                &p2t_nodes[p2t_nodes.size() - 3],
                &p2t_nodes[p2t_nodes.size() - 2],
                &p2t_nodes[p2t_nodes.size() - 1]});
        }
    }
    auto hole_contours = find_contours(hole_triangles, ContourDetectionStrategy::NODE_NEIGHBOR);

    std::vector<std::vector<p2t::Point>> p2t_hole_nodes;
    p2t_hole_nodes.reserve(hole_contours.size());
    for (const std::list<FixedArray<float, 3>>& c : hole_contours) {
        p2t_hole_nodes.push_back(std::vector<p2t::Point>());
        auto& pts = p2t_hole_nodes.back();
        pts.reserve(c.size());
        std::vector<p2t::Point*> hole_contour;
        hole_contour.reserve(c.size());
        // size_t i = 0;
        for (const auto& p : c) {
            pts.push_back(p2t::Point{p(0), p(1)});
            hole_contour.push_back(&pts.back());
            // draw_node(triangles, FixedArray<float, 2>{p(0), p(1)}, 0.1 * float(i++) / c.size());
        }
        cdt.AddHole(hole_contour);
    }
    std::list<p2t::Point> p2t_grid_nodes;
    for (const auto& p : steiner_points) {
        p2t_grid_nodes.push_back(p2t::Point{p.position(0), p.position(1)});
        cdt.AddPoint(&p2t_grid_nodes.back());
    }
    std::vector<p2t::Point> p2t_triangle_centers;
    p2t_triangle_centers.reserve(hole_triangles.size() * 3);
    std::set<p2t::Point*> p2t_hole_triangle_centers_set;
    for (const auto& t : hole_triangles) {
        auto add = [&](float a, float b, float c){
            auto center = a * t(0).position + b * t(1).position + c * t(2).position;
            p2t_triangle_centers.push_back(p2t::Point{center(0), center(1)});
            cdt.AddPoint(&*p2t_triangle_centers.rbegin());
            p2t_hole_triangle_centers_set.insert(&*p2t_triangle_centers.rbegin());
        };
        add(0.6f, 0.2f, 0.2f);
        add(0.2f, 0.6f, 0.2f);
        add(0.2f, 0.2f, 0.6f);
    }
    //triangles.clear();
    cdt.Triangulate();
    std::list<p2t::Triangle*> tris;
    if (hole_contours.empty()) {
        auto tris0 = cdt.GetTriangles();
        tris.insert(tris.end(), tris0.begin(), tris0.end());
    } else {
        auto tris0 = cdt.GetMap();
        tris.insert(tris.end(), tris0.begin(), tris0.end());
        std::list<PTri>& wrapped_tris = reinterpret_cast<std::list<PTri>&>(tris);
        delete_triangles_outside_contour(final_bounding_contour, wrapped_tris);
    }
    for (const auto& t : tris) {
        if (p2t_hole_triangle_centers_set.find(t->GetPoint(0)) != p2t_hole_triangle_centers_set.end()) {
            continue;
        }
        if (p2t_hole_triangle_centers_set.find(t->GetPoint(1)) != p2t_hole_triangle_centers_set.end()) {
            continue;
        }
        if (p2t_hole_triangle_centers_set.find(t->GetPoint(2)) != p2t_hole_triangle_centers_set.end()) {
            continue;
        }
        tl_terrain.draw_triangle_wo_normals(
            {float(t->GetPoint(0)->x), float(t->GetPoint(0)->y), z * scale},
            {float(t->GetPoint(1)->x), float(t->GetPoint(1)->y), z * scale},
            {float(t->GetPoint(2)->x), float(t->GetPoint(2)->y), z * scale},
            color,
            color,
            color,
            {float(t->GetPoint(0)->x) / scale * uv_scale, float(t->GetPoint(0)->y) / scale * uv_scale},
            {float(t->GetPoint(1)->x) / scale * uv_scale, float(t->GetPoint(1)->y) / scale * uv_scale},
            {float(t->GetPoint(2)->x) / scale * uv_scale, float(t->GetPoint(2)->y) / scale * uv_scale});
    }
    if (!tl_insert.empty() && (tl_terrain_visuals == nullptr)) {
        throw std::runtime_error("tl_insert without tl_terrain_visuals");
    }
    if (tl_terrain_visuals != nullptr) {
        for (const auto& l : tl_insert) {
            for (const auto& t : l) {
                tl_terrain_visuals->draw_triangle_wo_normals(
                    {t(0).position(0), t(0).position(1), z * scale},
                    {t(1).position(0), t(1).position(1), z * scale},
                    {t(2).position(0), t(2).position(1), z * scale},
                    color,
                    color,
                    color,
                    {t(0).position(0) / scale * uv_scale, t(0).position(1) / scale * uv_scale},
                    {t(1).position(0) / scale * uv_scale, t(1).position(1) / scale * uv_scale},
                    {t(2).position(0) / scale * uv_scale, t(2).position(1) / scale * uv_scale});
            }
        }
    }
}
