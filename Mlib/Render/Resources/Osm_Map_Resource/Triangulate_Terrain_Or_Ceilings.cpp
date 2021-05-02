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
#include <Mlib/Render/Resources/Osm_Map_Resource/Terrain_Uv.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Water_Type.hpp>
#include <poly2tri/point_exception.hpp>

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
            bvh.visit(BoundingSphere{edge}, [&edge, &highlighted_nodes](const Edge& data){
                FixedArray<float, 2> intersection;
                if (intersect_lines(intersection, edge, data, 0.f, 0.f, false, true)) {
                    highlighted_nodes.push_back({intersection(0), intersection(1), 0.f});
                }
                return true;
            });
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

class PointSet {
public:
    explicit PointSet(const std::list<SteinerPointInfo>& steiner_points) {
        for (const auto& p : steiner_points) {
            // Ignore result
            steiner_pts_.insert({
                OrderableFixedArray<float, 2>{p.position(0), p.position(1)},
                std::make_unique<p2t::Point>(p.position(0), p.position(1))});
        }
    }
    p2t::Point* operator () (float x, float y) {
        auto p = OrderableFixedArray<float, 2>{x, y};
        if (auto it = pts_.find(p); it != pts_.end()) {
            return it->second.get();
        }
        if (auto it = steiner_pts_.find(p); it != steiner_pts_.end()) {
            auto pt = it->second.get();
            pts_.insert({p, std::move(it->second)});
            steiner_pts_.erase(it);
            return pt;
        }
        auto pt = new p2t::Point{x, y};
        pts_.insert({p, std::unique_ptr<p2t::Point>(pt)});
        return pt;
    }
    std::vector<p2t::Point*> remaining_steiner_points() {
        std::vector<p2t::Point*> result;
        result.reserve(steiner_pts_.size());
        for (auto& p : steiner_pts_) {
            result.push_back(p.second.get());
        }
        return result;
    }
private:
    std::map<OrderableFixedArray<float, 2>, std::unique_ptr<p2t::Point>> pts_;
    std::map<OrderableFixedArray<float, 2>, std::unique_ptr<p2t::Point>> steiner_pts_;
};

template <class EntityType>
void triangulate_entity_list(
    EntityTypeTriangleList<EntityType>& tl_terrain,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const std::vector<FixedArray<float, 2>>& bounding_contour,
    const std::map<EntityType, std::list<FixedArray<ColoredVertex, 3>>>& hole_triangles,
    const std::list<std::pair<EntityType, std::list<FixedArray<float, 3>>>>& region_contours,
    float scale,
    float uv_scale,
    float z,
    const FixedArray<float, 3>& color,
    const std::string& contour_filename,
    const std::string& triangle_filename,
    EntityType default_terrain_type,
    const std::set<EntityType>& excluded_entitities)
{
    PointSet points{steiner_points};
    p2t::Point p00{bounding_info.boundary_min(0) - bounding_info.border_width, bounding_info.boundary_min(1) - bounding_info.border_width};
    p2t::Point p01{bounding_info.boundary_min(0) - bounding_info.border_width, bounding_info.boundary_max(1) + bounding_info.border_width};
    p2t::Point p10{bounding_info.boundary_max(0) + bounding_info.border_width, bounding_info.boundary_min(1) - bounding_info.border_width};
    p2t::Point p11{bounding_info.boundary_max(0) + bounding_info.border_width, bounding_info.boundary_max(1) + bounding_info.border_width};
    std::vector<p2t::Point*> p2t_bounding_contour;
    p2t_bounding_contour.reserve(bounding_contour.size());
    for (const auto& p : bounding_contour) {
        p2t_bounding_contour.push_back(points(p(0), p(1)));
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

    size_t ncontours = region_contours.size();
    std::map<EntityType, std::list<std::list<FixedArray<float, 3>>>> hole_contours;
    for (const auto& t : hole_triangles) {
        hole_contours[t.first] = find_contours(t.second, ContourDetectionStrategy::NODE_NEIGHBOR);
        ncontours += hole_contours[t.first].size();
    }
    std::vector<std::vector<p2t::Point*>> p2t_hole_contours;
    std::vector<EntityType> p2t_region_types;
    p2t_hole_contours.reserve(ncontours + 1);  // include bounding contour
    p2t_region_types.reserve(ncontours);

    p2t::CDT cdt{final_bounding_contour};
    auto add_contour = [&](EntityType region_type, const std::list<FixedArray<float, 3>>& contour){
        p2t_hole_contours.emplace_back();
        p2t_region_types.push_back(region_type);
        auto& cnt = p2t_hole_contours.back();
        cnt.reserve(contour.size());
        // size_t i = 0;
        for (const auto& p : contour) {
            cnt.push_back(points(p(0), p(1)));
            // draw_node(triangles, FixedArray<float, 2>{p(0), p(1)}, 0.1 * float(i++) / c.size());
        }
        check_contour(cnt);
        cdt.AddHole(cnt);
    };
    for (auto& hc : hole_contours) {
        for (std::list<FixedArray<float, 3>>& c : hc.second) {
            try {
                if (compute_area_ccw(c, scale) > 0) {
                    add_contour(hc.first, c);
                } else {
                    std::reverse(c.begin(), c.end());
                    add_contour(default_terrain_type, c);
                }
            } catch (const p2t::PointException& e) {
                throw p2t::PointException(e.point, "Could not add hole contour: " + std::string(e.what()));
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("Could not add hole contour: " + std::string(e.what()));
            }
        }
    }
    for (const auto& r : region_contours) {
        try {
            add_contour(r.first, r.second);
        } catch (const p2t::PointException& e) {
            throw p2t::PointException(e.point, "Could not add hole contour: " + std::string(e.what()));
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Could not add region contour: " + std::string(e.what()));
        }
    }
    for (const auto& p : points.remaining_steiner_points()) {
        cdt.AddPoint(p);
    }
    auto all_contours = p2t_hole_contours;
    all_contours.push_back(final_bounding_contour);
    if (!contour_filename.empty()) {
        plot_contours(contour_filename, all_contours);
    }
    //triangles.clear();
    cdt.Triangulate();
    std::list<p2t::Triangle*> tris;
    std::vector<std::list<p2t::Triangle*>> inner_triangles;
    if (ncontours == 0) {
        auto tris0 = cdt.GetTriangles();
        tris.insert(tris.end(), tris0.begin(), tris0.end());
    } else {
        auto tris0 = cdt.GetMap();
        tris.insert(tris.end(), tris0.begin(), tris0.end());
        if (!triangle_filename.empty()) {
            plot_tris(triangle_filename, tris);
        }
        std::list<PTri>* wrapped_tris = reinterpret_cast<std::list<PTri>*>(&tris);
        std::vector<std::list<PTri>>* wrapped_itris = reinterpret_cast<std::vector<std::list<PTri>>*>(&inner_triangles);
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
                terrain_uv(float(t->GetPoint(0)->x), float(t->GetPoint(0)->y), scale, uv_scale),
                terrain_uv(float(t->GetPoint(1)->x), float(t->GetPoint(1)->y), scale, uv_scale),
                terrain_uv(float(t->GetPoint(2)->x), float(t->GetPoint(2)->y), scale, uv_scale));
        }
    };
    if (ncontours == 0) {
        draw_tris(tl_terrain[default_terrain_type], tris);
    } else {
        if (inner_triangles.empty()) {
            throw std::runtime_error("Triangulate internal error");
        }
        for (size_t i = 0; i < inner_triangles.size() - 1; ++i) {
            if (!excluded_entitities.contains(p2t_region_types[i])) {
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
    const std::map<TerrainType, std::list<FixedArray<ColoredVertex, 3>>>& hole_triangles,
    const std::list<std::pair<TerrainType, std::list<FixedArray<float, 3>>>>& region_contours,
    float scale,
    float uv_scale,
    float z,
    const FixedArray<float, 3>& color,
    const std::string& contour_filename,
    const std::string& triangle_filename,
    TerrainType default_terrain_type,
    const std::set<TerrainType>& excluded_terrain_types)
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
        triangle_filename,
        default_terrain_type,
        excluded_terrain_types);
}

void Mlib::triangulate_water(
    WaterTypeTriangleList& tl_water,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const std::vector<FixedArray<float, 2>>& bounding_contour,
    const std::map<WaterType, std::list<FixedArray<ColoredVertex, 3>>>& hole_triangles,
    const std::list<std::pair<WaterType, std::list<FixedArray<float, 3>>>>& region_contours,
    float scale,
    float uv_scale,
    float z,
    const FixedArray<float, 3>& color,
    const std::string& contour_filename,
    const std::string& triangle_filename,
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
        triangle_filename,
        default_water_type,
        {WaterType::HOLE});
}
