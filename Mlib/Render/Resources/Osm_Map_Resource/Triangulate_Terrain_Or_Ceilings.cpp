#include "Triangulate_Terrain_Or_Ceilings.hpp"
#include <Mlib/Geometry/Mesh/Contour.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Bounding_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Triangle_Lists.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Terrain_Type.hpp>
#include <poly2tri/poly2tri.h>
#include <Mlib/Geometry/Mesh/Save_Obj.hpp>
#include <Mlib/Geometry/Mesh/Indexed_Face_Set.hpp>

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
    TerrainTypeTriangleList& tl_terrain,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const std::vector<FixedArray<float, 2>>& bounding_contour,
    const std::list<FixedArray<ColoredVertex, 3>>& hole_triangles,
    const std::map<TerrainType, std::list<FixedArray<float, 3>>>& region_contours,
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
    std::vector<std::vector<p2t::Point*>> p2t_hole_contours;
    std::vector<TerrainType> p2t_region_types;
    p2t_hole_nodes.reserve(hole_contours.size() + region_contours.size());
    p2t_hole_contours.reserve(p2t_hole_nodes.size());
    p2t_region_types.reserve(p2t_hole_nodes.size());
    auto add_contour = [&p2t_hole_nodes, &p2t_hole_contours, &p2t_region_types, &cdt](TerrainType region_type, const std::list<FixedArray<float, 3>>& contour){
        p2t_hole_nodes.push_back(std::vector<p2t::Point>());
        p2t_hole_contours.push_back(std::vector<p2t::Point*>());
        p2t_region_types.push_back(region_type);
        auto& pts = p2t_hole_nodes.back();
        auto& cnt = p2t_hole_contours.back();
        pts.reserve(contour.size());
        cnt.reserve(contour.size());
        // size_t i = 0;
        for (const auto& p : contour) {
            pts.push_back(p2t::Point{p(0), p(1)});
            cnt.push_back(&pts.back());
            // draw_node(triangles, FixedArray<float, 2>{p(0), p(1)}, 0.1 * float(i++) / c.size());
        }
        cdt.AddHole(cnt);
    };
    for (const std::list<FixedArray<float, 3>>& c : hole_contours) {
        add_contour(TerrainType::HOLE, c);
    }
    for (const auto& r : region_contours) {
        add_contour(r.first, r.second);
    }
    std::list<p2t::Point> p2t_grid_nodes;
    for (const auto& p : steiner_points) {
        p2t_grid_nodes.push_back(p2t::Point{p.position(0), p.position(1)});
        cdt.AddPoint(&p2t_grid_nodes.back());
    }
    //triangles.clear();
    cdt.Triangulate();
    std::list<p2t::Triangle*> tris;
    std::vector<std::list<p2t::Triangle*>> inner_triangles;
    if (p2t_hole_nodes.empty()) {
        auto tris0 = cdt.GetTriangles();
        tris.insert(tris.end(), tris0.begin(), tris0.end());
    } else {
        auto tris0 = cdt.GetMap();
        tris.insert(tris.end(), tris0.begin(), tris0.end());
        std::list<PTri>& wrapped_tris = reinterpret_cast<std::list<PTri>&>(tris);
        std::vector<std::list<PTri>>& wrapped_itris = reinterpret_cast<std::vector<std::list<PTri>>&>(inner_triangles);
        auto all_contours = p2t_hole_contours;
        all_contours.push_back(final_bounding_contour);
        delete_triangles_inside_contours(all_contours, wrapped_tris, wrapped_itris);
        std::list<FixedArray<ColoredVertex, 3>> triangles;
        for (const auto& t : wrapped_tris) {
            triangles.push_back(FixedArray<ColoredVertex, 3>{
                ColoredVertex{.position = {(float)t(0)->x, (float)t(0)->y, 0.f}, .color = {1.f, 1.f, 1.f}, .uv = {0.f, 0.f}, .normal = {0.f, 0.f, 1.f}, .tangent = {0.f, 1.f, 0.f}},
                ColoredVertex{.position = {(float)t(1)->x, (float)t(1)->y, 0.f}, .color = {1.f, 1.f, 1.f}, .uv = {0.f, 0.f}, .normal = {0.f, 0.f, 1.f}, .tangent = {0.f, 1.f, 0.f}},
                ColoredVertex{.position = {(float)t(2)->x, (float)t(2)->y, 0.f}, .color = {1.f, 1.f, 1.f}, .uv = {0.f, 0.f}, .normal = {0.f, 0.f, 1.f}, .tangent = {0.f, 1.f, 0.f}}
            });
        }
        save_obj("/tmp/wrapped_tris1.obj", IndexedFaceSet<float, size_t>{triangles});
    }
    auto draw_tris = [z, scale, color, uv_scale](auto& tl, const auto& tris){
        for (const auto& t : tris) {
            tl->draw_triangle_wo_normals(
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
    };
    if (p2t_hole_nodes.empty()) {
        draw_tris(tl_terrain[TerrainType::UNDEFINED], tris);
    } else {
        if (inner_triangles.empty()) {
            throw std::runtime_error("Triangulate internal error");
        }
        for (size_t i = 0; i < inner_triangles.size() - 1; ++i) {
            draw_tris(tl_terrain[p2t_region_types[i]], inner_triangles[i]);
        }
        draw_tris(tl_terrain[TerrainType::UNDEFINED], inner_triangles.back());
    }
}
