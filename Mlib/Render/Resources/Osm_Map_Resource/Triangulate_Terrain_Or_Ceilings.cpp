#include "Triangulate_Terrain_Or_Ceilings.hpp"
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Intersect_Lines.hpp>
#include <Mlib/Geometry/Mesh/Contour.hpp>
#include <Mlib/Geometry/Mesh/Indexed_Face_Set.hpp>
#include <Mlib/Geometry/Mesh/Plot.hpp>
#include <Mlib/Geometry/Mesh/Save_Obj.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Bounding_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Triangle_Lists.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/PTri.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Terrain_Type.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Water_Type.hpp>

using namespace Mlib;

void plot_tris(const std::string& filename, const std::list<p2t::Triangle*>& tris) {
    std::list<FixedArray<ColoredVertex, 3>> triangles;
    for (const auto& t : tris) {
        triangles.push_back(FixedArray<ColoredVertex, 3>{
            ColoredVertex{.position = {(float)t->GetPoint(0)->x, (float)t->GetPoint(0)->y, 0.f}, .color = {1.f, 1.f, 1.f}, .uv = {0.f, 0.f}, .normal = {0.f, 0.f, 1.f}, .tangent = {0.f, 1.f, 0.f}},
            ColoredVertex{.position = {(float)t->GetPoint(1)->x, (float)t->GetPoint(1)->y, 0.f}, .color = {1.f, 1.f, 1.f}, .uv = {0.f, 0.f}, .normal = {0.f, 0.f, 1.f}, .tangent = {0.f, 1.f, 0.f}},
            ColoredVertex{.position = {(float)t->GetPoint(2)->x, (float)t->GetPoint(2)->y, 0.f}, .color = {1.f, 1.f, 1.f}, .uv = {0.f, 0.f}, .normal = {0.f, 0.f, 1.f}, .tangent = {0.f, 1.f, 0.f}}
        });
    }
    save_obj(filename, IndexedFaceSet<float, size_t>{triangles});
}

void plot_contours(const std::string& filename, const std::vector<std::vector<p2t::Point*>>& p2t_hole_contours) {
    std::list<std::list<FixedArray<float, 3>>> contours;
    std::list<FixedArray<float, 3>> highlighted_nodes;
    for (const auto& c : p2t_hole_contours) {
        contours.emplace_back();
        for (const auto& p : c) {
            contours.back().push_back({(float)p->x, -(float)p->y, 0.f});
        }
        contours.back().push_back({(float)c.front()->x, -(float)c.front()->y, 0.f});
    }
    typedef FixedArray<float, 2> P2;
    typedef FixedArray<P2, 2> Edge;
    Bvh<float, Edge, 2> bvh{{0.1f, 0.1f}, 10};
    for (const auto& c : p2t_hole_contours) {
        for (auto it = c.begin(); it != c.end(); ++it) {
            auto s = it;
            ++s;
            auto edge = Edge{
                P2{(float)(*it)->x, -(float)(*it)->y},
                (s == c.end())
                    ? P2{(float)c.front()->x, -(float)c.front()->y}
                    : P2{(float)(*s)->x, -(float)(*s)->y}};
            if (bvh.visit(BoundingSphere{edge}, [&edge, &highlighted_nodes](const Edge& data){
                FixedArray<float, 2> intersection;
                if (intersect_lines(intersection, edge, data, 0.f, 0.f, false, true)) {
                    highlighted_nodes.push_back({intersection(0), intersection(1), 0.f});
                }
                return true;
            }));
            bvh.insert(edge, edge);
        }
    }
    plot_mesh_svg(
        filename,
        600,
        500,
        {},
        contours,
        highlighted_nodes);
}

float compute_area_ccw(
    const std::vector<p2t::Point*>& polygon,
    float scale)
{
    // Source: https://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
    float area2 = 0;
    for (auto it = polygon.begin(); it != polygon.end(); ++it) {
        auto s = it;
        ++s;
        const p2t::Point& a = **it;
        const p2t::Point& b = (s != polygon.end()) ? **s : *polygon.front();
        area2 += (a.x - b.x) * (b.y + a.y);
    }
    return area2 / 2 / squared(scale);
}

float compute_area_ccw(
    const std::list<FixedArray<float, 3>>& polygon,
    float scale)
{
    // Source: https://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
    float area2 = 0;
    for (auto it = polygon.begin(); it != polygon.end(); ++it) {
        auto s = it;
        ++s;
        const auto& a = *it;
        const auto& b = (s != polygon.end()) ? **s : *polygon.front();
        area2 += (a(0) - b(0)) * (b(1) + a(1));
    }
    return area2 / 2 / squared(scale);
}

void check_contour(const std::vector<p2t::Point*>& contour) {
    if ((contour.front()->x == contour.back()->x) &&
        (contour.front()->y == contour.back()->y))
    {
        throw std::runtime_error("Triangulation boundary contour is closed");
    }
    if (compute_area_ccw(contour, 1.f) < 0.f) {
        throw std::runtime_error("Contour is not counterclockwise");
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

template <class EntityType>
void triangulate_entity_list(
    EntityTypeTriangleList<EntityType>& tl_terrain,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const std::vector<FixedArray<float, 2>>& bounding_contour,
    const std::list<FixedArray<ColoredVertex, 3>>& hole_triangles,
    const std::list<std::pair<EntityType, std::list<FixedArray<float, 3>>>>& region_contours,
    float scale,
    float uv_scale,
    float z,
    const FixedArray<float, 3>& color,
    const std::string& contour_filename,
    EntityType default_terrain_type)
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
    std::reverse(final_bounding_contour.begin(), final_bounding_contour.end());
    // for (const auto& p : final_bounding_contour) {
    //     std::cerr << "c " << p->x << " " << p->y << std::endl;
    // }
    try {
        check_contour(final_bounding_contour);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("Could not add bounding contour: " + std::string(e.what()));
    }
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
    std::vector<EntityType> p2t_region_types;
    p2t_hole_nodes.reserve(hole_contours.size() + region_contours.size());
    p2t_hole_contours.reserve(p2t_hole_nodes.size());
    p2t_region_types.reserve(p2t_hole_nodes.size());
    auto add_contour = [&p2t_hole_nodes, &p2t_hole_contours, &p2t_region_types, &cdt](EntityType region_type, const std::list<FixedArray<float, 3>>& contour){
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
        check_contour(cnt);
        cdt.AddHole(cnt);
    };
    for (std::list<FixedArray<float, 3>>& c : hole_contours) {
        try {
            if (compute_area_ccw(c, scale) > 0) {
                add_contour(EntityType::HOLE, c);
            } else {
                std::reverse(c.begin(), c.end());
                add_contour(default_terrain_type, c);
            }
        } catch (std::runtime_error& e) {
            throw std::runtime_error("Could not add hole contour: " + std::string(e.what()));
        }
    }
    for (const auto& r : region_contours) {
        try {
            add_contour(r.first, r.second);
        } catch (std::runtime_error& e) {
            throw std::runtime_error("Could not add region contour: " + std::string(e.what()));
        }
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
        // plot_tris("/tmp/wrapped_tris0.obj", tris);
        std::list<PTri>* wrapped_tris = reinterpret_cast<std::list<PTri>*>(&tris);
        std::vector<std::list<PTri>>* wrapped_itris = reinterpret_cast<std::vector<std::list<PTri>>*>(&inner_triangles);
        auto all_contours = p2t_hole_contours;
        all_contours.push_back(final_bounding_contour);
        if (!contour_filename.empty()) {
            plot_contours(contour_filename, all_contours);
        }
        delete_triangles_inside_contours(all_contours, *wrapped_tris, *wrapped_itris);
        // plot_tris("/tmp/wrapped_tris1.obj", tris);
        // save_obj("/tmp/holes.obj", IndexedFaceSet<float, size_t>{hole_triangles});
        // std::cerr << "nresidual " << wrapped_tris.size() << std::endl;
        // for (const auto& l : wrapped_itris) {
        //     std::cerr << "ninner " << l.size() << std::endl;
        // }
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
        draw_tris(tl_terrain[default_terrain_type], tris);
    } else {
        if (inner_triangles.empty()) {
            throw std::runtime_error("Triangulate internal error");
        }
        for (size_t i = 0; i < inner_triangles.size() - 1; ++i) {
            if (p2t_region_types[i] != EntityType::HOLE) {
                draw_tris(tl_terrain[p2t_region_types[i]], inner_triangles[i]);
            }
        }
        draw_tris(tl_terrain[default_terrain_type], inner_triangles.back());
    }
    // for (const auto& l : tl_terrain.map()) {
    //     save_obj("/tmp/" + to_string(l.first) + ".obj", IndexedFaceSet<float, size_t>{l.second->triangles_});
    //     for (const auto& t : l.second->triangles_) {
    //         std::cerr << t << std::endl;
    //     }
    // }
}

void Mlib::triangulate_terrain_or_ceilings(
    TerrainTypeTriangleList& tl_terrain,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const std::vector<FixedArray<float, 2>>& bounding_contour,
    const std::list<FixedArray<ColoredVertex, 3>>& hole_triangles,
    const std::list<std::pair<TerrainType, std::list<FixedArray<float, 3>>>>& region_contours,
    float scale,
    float uv_scale,
    float z,
    const FixedArray<float, 3>& color,
    const std::string& contour_filename,
    TerrainType default_terrain_type)
{
    triangulate_entity_list(
        tl_terrain,
        bounding_info,
        steiner_points,
        bounding_contour,
        hole_triangles,
        region_contours,
        scale,
        uv_scale,
        z,
        color,
        contour_filename,
        default_terrain_type);
}

void Mlib::triangulate_water(
    WaterTypeTriangleList& tl_water,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const std::vector<FixedArray<float, 2>>& bounding_contour,
    const std::list<FixedArray<ColoredVertex, 3>>& hole_triangles,
    const std::list<std::pair<WaterType, std::list<FixedArray<float, 3>>>>& region_contours,
    float scale,
    float uv_scale,
    float z,
    const FixedArray<float, 3>& color,
    const std::string& contour_filename,
    WaterType default_water_type)
{
    triangulate_entity_list(
        tl_water,
        bounding_info,
        steiner_points,
        bounding_contour,
        hole_triangles,
        region_contours,
        scale,
        uv_scale,
        z,
        color,
        contour_filename,
        default_water_type);
}
