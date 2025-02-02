#include "Triangulate_Terrain_Or_Ceilings.hpp"
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Geometry/Exceptions/Point_Exception.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Intersect_Lines.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Close_Neighbor_Detector.hpp>
#include <Mlib/Geometry/Mesh/Contour.hpp>
#include <Mlib/Geometry/Mesh/Indexed_Face_Set.hpp>
#include <Mlib/Geometry/Mesh/P2t_Point_Set.hpp>
#include <Mlib/Geometry/Mesh/Plot.hpp>
#include <Mlib/Geometry/Mesh/Save_Obj.hpp>
#include <Mlib/Geometry/Mesh/Terrain_Uv.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Largest_Cosine.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Bounding_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Triangle_Lists.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/PTri.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Water_Type.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Type.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <poly2tri/point_exception.hpp>

using namespace Mlib;

void plot_tris(const std::string& filename, const std::list<p2t::Triangle*>& tris) {
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

void plot_tris(
    const std::string& filename,
    const std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& tris,
    const std::set<OrderableFixedArray<CompressedScenePos, 3>>& highlighted_points)
{
    std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> standard_triangles;
    std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> highlighted_triangles;
    for (const auto& t : tris) {
        bool is_highlighted = false;
        for (auto& v : t.flat_iterable()) {
            if (highlighted_points.contains(OrderableFixedArray{v.position})) {
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

static void plot_contours(const std::string& filename, const std::vector<std::vector<p2t::Point*>>& p2t_hole_contours, double scale) {
    std::list<std::list<FixedArray<CompressedScenePos, 3>>> contours;
    std::list<FixedArray<CompressedScenePos, 3>> highlighted_nodes;
    for (const auto& c : p2t_hole_contours) {
        auto& cs = contours.emplace_back();
        for (const auto& p : c) {
            cs.emplace_back(
                (CompressedScenePos)(p->x / scale),
                (CompressedScenePos)(-p->y / scale),
                (CompressedScenePos)0.);
        }
        cs.emplace_back(
            (CompressedScenePos)(c.front()->x / scale),
            (CompressedScenePos)(-c.front()->y / scale),
            (CompressedScenePos)0.);
    }
    typedef FixedArray<CompressedScenePos, 2> P2;
    typedef FixedArray<CompressedScenePos, 2, 2> Edge;
    Bvh<CompressedScenePos, 2, Edge> bvh{{(CompressedScenePos)100., (CompressedScenePos)100.}, 10};
    for (const auto& c : p2t_hole_contours) {
        for (auto it = c.begin(); it != c.end(); ++it) {
            auto s = it;
            ++s;
            auto edge = Edge{
                P2{(CompressedScenePos)((*it)->x / scale), (CompressedScenePos)(-(*it)->y / scale)},
                (s == c.end())
                    ? P2{(CompressedScenePos)(c.front()->x / scale), (CompressedScenePos)(-c.front()->y / scale)}
                    : P2{(CompressedScenePos)((*s)->x / scale), (CompressedScenePos)(-(*s)->y / scale)}};
            auto aabb = AxisAlignedBoundingBox<CompressedScenePos, 2>::from_points(edge);
            bvh.visit(
                aabb,
                [&edge, &highlighted_nodes](const Edge& data)
            {
                FixedArray<double, 2> intersection = uninitialized;
                if (intersect_lines(intersection, funpack(edge), funpack(data), 0., 0., false, true)) {
                    highlighted_nodes.emplace_back((CompressedScenePos)intersection(0), (CompressedScenePos)intersection(1), (CompressedScenePos)0.f);
                }
                return true;
            });
            bvh.insert(aabb, edge);
        }
    }
    plot_mesh_svg(
        filename,
        600.,
        500.,
        {},
        contours,
        highlighted_nodes,
        (CompressedScenePos)0.1f); // line_width
}

double compute_area_ccw(
    const std::vector<p2t::Point*>& polygon,
    double scale)
{
    // Source: https://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
    double area2 = 0;
    for (auto it = polygon.begin(); it != polygon.end(); ++it) {
        auto s = it;
        ++s;
        const p2t::Point& a = **it;
        const p2t::Point& b = (s != polygon.end()) ? **s : *polygon.front();
        area2 += (a.x - b.x) * (b.y + a.y);
    }
    return area2 / 2. / squared(scale);
}

double compute_area_ccw(
    const std::list<FixedArray<CompressedScenePos, 2>>& polygon,
    double scale)
{
    // Source: https://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
    double area2 = 0;
    for (auto it = polygon.begin(); it != polygon.end(); ++it) {
        auto s = it;
        ++s;
        const auto& a = *it;
        const auto& b = (s != polygon.end()) ? **s : *polygon.front();
        area2 += funpack(a(0) - b(0)) * funpack(b(1) + a(1));
    }
    return area2 / 2. / squared(scale);
}

void check_contour(const std::vector<p2t::Point*>& contour) {
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

template <class EntityType>
void triangulate_entity_list(
    EntityTypeTriangleList<EntityType>& tl_terrain,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const UUVector<FixedArray<CompressedScenePos, 2>>& bounding_contour,
    const std::map<EntityType, std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>>& hole_triangles,
    const std::list<std::pair<EntityType, std::list<FixedArray<CompressedScenePos, 2>>>>& region_contours,
    float scale,
    float triangulation_scale,
    float uv_scale,
    float uv_period,
    CompressedScenePos z,
    const FixedArray<float, 3>& color,
    const std::string& contour_triangles_filename,
    const std::string& contour_filename,
    const std::string& triangle_filename,
    EntityType bounding_terrain_type,
    EntityType default_terrain_type,
    const std::set<EntityType>& excluded_entitities,
    ContourDetectionStrategy contour_detection_strategy)
{
    std::list<FixedArray<CompressedScenePos, 2>> steiner_point_positions;
    for (const auto& p : steiner_points) {
        steiner_point_positions.emplace_back(p.position(0), p.position(1));
    }
    P2tPointSet points{ steiner_point_positions, triangulation_scale };
    std::vector<p2t::Point*> final_bounding_contour;
    if (bounding_contour.empty()) {
        final_bounding_contour = {
            points({bounding_info.boundary_min(0) - bounding_info.border_width, bounding_info.boundary_min(1) - bounding_info.border_width}),
            points({bounding_info.boundary_min(0) - bounding_info.border_width, bounding_info.boundary_max(1) + bounding_info.border_width}),
            points({bounding_info.boundary_max(0) + bounding_info.border_width, bounding_info.boundary_max(1) + bounding_info.border_width}),
            points({bounding_info.boundary_max(0) + bounding_info.border_width, bounding_info.boundary_min(1) - bounding_info.border_width}) };
    } else {
        final_bounding_contour.reserve(bounding_contour.size());
        for (const auto& p : bounding_contour) {
            final_bounding_contour.push_back(points(p));
        }
    }
    if (compute_area_ccw(final_bounding_contour, scale) < 0.f) {
        std::reverse(final_bounding_contour.begin(), final_bounding_contour.end());
    }
    // for (const auto& p : final_bounding_contour) {
    //     lerr() << "c " << p->x << " " << p->y;
    // }
    try {
        check_contour(final_bounding_contour);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("Could not add bounding contour: " + std::string(e.what()));
    }

    size_t ncontours = region_contours.size();
    std::map<EntityType, std::list<std::list<FixedArray<CompressedScenePos, 2>>>> hole_contours;
    CloseNeighborDetector<CompressedScenePos, 2> close_neighbor_detector{{(CompressedScenePos)10., (CompressedScenePos)10.}, 10};
    for (const auto& [e, t] : hole_triangles) {
        // save_obj("/tmp/" + to_string(e) + ".obj", IndexedFaceSet<float, CompressedScenePos, size_t>{t}, nullptr);
        for (const auto& tt : t) {
            for (const auto& v : tt.flat_iterable()) {
                if (close_neighbor_detector.contains_neighbor(
                    {v.position(0), v.position(1)},
                    (CompressedScenePos)1e-2))
                {
                    if (!contour_triangles_filename.empty()) {
                        plot_tris(contour_triangles_filename + to_string(e) + ".current.obj", t, { OrderableFixedArray{v.position} });
                        for (const auto& [e1, t1] : hole_triangles) {
                            plot_tris(contour_triangles_filename + to_string(e1) + ".other.obj", t1, { OrderableFixedArray{v.position} });
                        }
                    }
                    auto exception = PointException<CompressedScenePos, 2>{
                        {v.position(0), v.position(1)},
                        "Detected near-duplicate point"};
                    THROW_OR_ABORT2(exception);
                }
            }
            {
                using Triangle = FixedArray<CompressedScenePos, 3, 3>;
                auto tlc = triangle_largest_cosine(funpack(Triangle{
                    tt(0).position,
                    tt(1).position,
                    tt(2).position}));
                if (std::isnan(tlc) || (tlc > (1 - 1e-7f))) {
                    auto exception = TriangleException<CompressedScenePos>{
                        tt(0).position, tt(1).position, tt(2).position,
                        "Detected bad triangle"};
                    THROW_OR_ABORT2(exception);
                }
            }
        }
        try {
            auto cs3 = find_contours(t, contour_detection_strategy);
            auto& cs2 = hole_contours[e];
            for (const auto& c3 : cs3) {
                auto& c2 = cs2.emplace_back();
                for (const auto& p : c3) {
                    c2.push_back({p(0), p(1)});
                }
            }
        } catch (const EdgeException<CompressedScenePos>& ex) {
            if (!contour_triangles_filename.empty()) {
                plot_tris(contour_triangles_filename, t, { OrderableFixedArray{ex.a}, OrderableFixedArray{ex.b} });
            }
            throw;
        }
        ncontours += hole_contours[e].size();
    }
    std::vector<std::vector<p2t::Point*>> p2t_hole_contours;
    std::vector<EntityType> p2t_region_types;
    p2t_hole_contours.reserve(ncontours + 1);  // include bounding contour
    p2t_region_types.reserve(ncontours);

    p2t::CDT cdt{final_bounding_contour};
    auto add_contour = [&](EntityType region_type, const std::list<FixedArray<CompressedScenePos, 2>>& contour){
        p2t_hole_contours.emplace_back();
        p2t_region_types.push_back(region_type);
        auto& cnt = p2t_hole_contours.back();
        cnt.reserve(contour.size());
        // size_t i = 0;
        for (const auto& p : contour) {
            cnt.push_back(points(p));
            // draw_node(triangles, p.casted<float>(), 0.1 * float(i++) / c.size());
        }
        check_contour(cnt);
        cdt.AddHole(cnt);
    };
    for (auto& hc : hole_contours) {
        for (std::list<FixedArray<CompressedScenePos, 2>>& c : hc.second) {
            try {
                if (compute_area_ccw(c, scale) > 0) {
                    add_contour(hc.first, c);
                } else {
                    c.reverse();
                    add_contour(default_terrain_type, c);
                }
            } catch (const p2t::PointException& e) {
                throw p2t::PointException(e.point, "Could not add hole contour: " + std::string(e.what()));
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("Could not add hole contour: " + std::string(e.what()));
            }
        }
    }
    for (const auto& [tpe, c] : region_contours) {
        try {
            add_contour(tpe, c);
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
        plot_contours(contour_filename, all_contours, scale * triangulation_scale);
    }
    //triangles.clear();
    cdt.Triangulate();
    std::list<p2t::Triangle*> tris;
    std::vector<std::list<p2t::Triangle*>> inner_triangles;
    if (ncontours == 0) {
        auto tris0 = cdt.GetTriangles();
        tris.insert(tris.end(), tris0.begin(), tris0.end());
        if (!triangle_filename.empty()) {
            plot_tris(triangle_filename, tris);
        }
    } else {
        auto tris0 = cdt.GetMap();
        tris.insert(tris.end(), tris0.begin(), tris0.end());
        if (!triangle_filename.empty()) {
            plot_tris(triangle_filename, tris);
        }
        std::list<PTri>* wrapped_tris = reinterpret_cast<std::list<PTri>*>(&tris);
        std::vector<std::list<PTri>>* wrapped_itris = reinterpret_cast<std::vector<std::list<PTri>>*>(&inner_triangles);
        extract_triangles_inside_contours(all_contours, *wrapped_tris, *wrapped_itris);
        // plot_tris("/tmp/wrapped_tris1.obj", tris);
        // save_obj("/tmp/holes.obj", IndexedFaceSet<float, size_t>{hole_triangles});
        // lerr() << "nresidual " << wrapped_tris.size();
        // for (const auto& l : wrapped_itris) {
        //     lerr() << "ninner " << l.size();
        // }
    }
    auto draw_tris = [&](auto& tl, const auto& tris){
        for (const auto& t : tris) {
            const auto* c0 = points.try_get_coords(t->GetPoint(0));
            const auto* c1 = points.try_get_coords(t->GetPoint(1));
            const auto* c2 = points.try_get_coords(t->GetPoint(2));
            if ((c0 == nullptr) ||
                (c1 == nullptr) ||
                (c2 == nullptr))
            {
                lwarn() << "Received unknown point";
                continue;
            }
            auto uv = terrain_uv<CompressedScenePos, double>(
                *c0,
                *c1,
                *c2,
                scale,
                uv_scale,
                uv_period);
            tl->draw_triangle_wo_normals(
                {(*c0)(0), (*c0)(1), z},
                {(*c1)(0), (*c1)(1), z},
                {(*c2)(0), (*c2)(1), z},
                Colors::from_rgb(color),
                Colors::from_rgb(color),
                Colors::from_rgb(color),
                uv[0],
                uv[1],
                uv[2]);
        }
    };
    if (ncontours == 0) {
        draw_tris(tl_terrain[default_terrain_type], tris);
    } else {
        if (inner_triangles.empty()) {
            THROW_OR_ABORT("Triangulate internal error");
        }
        for (size_t i = 0; i < inner_triangles.size() - 1; ++i) {
            if (!excluded_entitities.contains(p2t_region_types[i])) {
                draw_tris(tl_terrain[p2t_region_types[i]], inner_triangles[i]);
            }
        }
        draw_tris(tl_terrain[bounding_terrain_type], inner_triangles.back());
    }
    // for (const auto& l : tl_terrain.map()) {
    //     save_obj("/tmp/" + to_string(l.first) + ".obj", IndexedFaceSet<float, size_t>{l.second->triangles_});
    //     for (const auto& t : l.second->triangles_) {
    //         lerr() << t;
    //     }
    // }
}

void Mlib::triangulate_terrain_or_ceilings(
    TerrainTypeTriangleList& tl_terrain,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const UUVector<FixedArray<CompressedScenePos, 2>>& bounding_contour,
    const std::map<TerrainType, std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>>& hole_triangles,
    const std::list<std::pair<TerrainType, std::list<FixedArray<CompressedScenePos, 2>>>>& region_contours,
    float scale,
    float triangulation_scale,
    float uv_scale,
    float uv_period,
    CompressedScenePos z,
    const FixedArray<float, 3>& color,
    const std::string& contour_triangles_filename,
    const std::string& contour_filename,
    const std::string& triangle_filename,
    TerrainType bounding_terrain_type,
    TerrainType default_terrain_type,
    const std::set<TerrainType>& excluded_terrain_types,
    ContourDetectionStrategy contour_detection_strategy)
{
    triangulate_entity_list(
        tl_terrain,
        bounding_info,
        steiner_points,
        bounding_contour,
        hole_triangles,
        region_contours,
        scale,
        triangulation_scale,
        uv_scale,
        uv_period,
        z,
        color,
        contour_triangles_filename,
        contour_filename,
        triangle_filename,
        bounding_terrain_type,
        default_terrain_type,
        excluded_terrain_types,
        contour_detection_strategy);
}

void Mlib::triangulate_water(
    WaterTypeTriangleList& tl_water,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const UUVector<FixedArray<CompressedScenePos, 2>>& bounding_contour,
    const std::map<WaterType, std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>>& hole_triangles,
    const std::list<std::pair<WaterType, std::list<FixedArray<CompressedScenePos, 2>>>>& region_contours,
    float scale,
    float triangulation_scale,
    float uv_scale,
    float uv_period,
    CompressedScenePos z,
    const FixedArray<float, 3>& color,
    const std::string& contour_triangles_filename,
    const std::string& contour_filename,
    const std::string& triangle_filename,
    WaterType bounding_water_type,
    WaterType default_water_type,
    ContourDetectionStrategy contour_detection_strategy)
{
    triangulate_entity_list(
        tl_water,
        bounding_info,
        steiner_points,
        bounding_contour,
        hole_triangles,
        region_contours,
        scale,
        triangulation_scale,
        uv_scale,
        uv_period,
        z,
        color,
        contour_triangles_filename,
        contour_filename,
        triangle_filename,
        bounding_water_type,
        default_water_type,
        { WaterType::HOLE },  // excluded_entitities
        contour_detection_strategy);
}
