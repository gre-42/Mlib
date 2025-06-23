#pragma once
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>
#include <Mlib/Geometry/Exceptions/Point_Exception.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Close_Neighbor_Detector.hpp>
#include <Mlib/Geometry/Mesh/Contour.hpp>
#include <Mlib/Geometry/Mesh/P2t_Point_Set.hpp>
#include <Mlib/Geometry/Mesh/Terrain_Uv.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Largest_Cosine.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Bounding_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Compute_Area.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Get_Region_Margin_Contour.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Triangle_Lists.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/PTri.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Region_With_Margin.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Subdivided_Contour.hpp>
#include <list>
#include <poly2tri/point_exception.hpp>
#include <string>

namespace Mlib {

static const float BAD_TRIANGLE_COS = 1 - 1e-8f;

void plot_tris(const std::string& filename, const std::list<p2t::Triangle*>& tris);

void plot_tris(
    const std::string& filename,
    const std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& tris,
    const std::set<OrderableFixedArray<CompressedScenePos, 3>>& highlighted_points);

void plot_contours(const std::string& filename, const std::vector<std::vector<p2t::Point*>>& p2t_hole_contours, double scale);

void check_contour(const std::vector<p2t::Point*>& contour);

template <class EntityType>
void triangulate_entity_list(
    EntityTypeTriangleList<EntityType>& tl_terrain,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const std::vector<FixedArray<CompressedScenePos, 2>>& bounding_contour,
    const std::list<RegionWithMargin<EntityType, std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>>>& hole_triangles,
    const std::list<RegionWithMargin<EntityType, std::list<FixedArray<CompressedScenePos, 2>>>>& region_contours,
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
    ContourDetectionStrategy contour_detection_strategy,
    const std::map<OrderableFixedArray<CompressedScenePos, 2>, CompressedScenePos>& garden_margin)
{
    std::list<FixedArray<CompressedScenePos, 2>> steiner_point_positions;
    for (const auto& p : steiner_points) {
        steiner_point_positions.emplace_back(p.position(0), p.position(1));
    }
    P2tPointSet points{ steiner_point_positions, triangulation_scale };
    std::vector<p2t::Point*> final_bounding_contour;
    auto compute_final_bounding_contour = [&](const auto& bc){
        final_bounding_contour.reserve(bc.size());
        for (const auto& p : bc) {
            final_bounding_contour.push_back(points(p));
        }
    };
    if (bounding_contour.empty()) {
        std::list<FixedArray<CompressedScenePos, 2>> bc0 = {
            {bounding_info.boundary_min(0) - bounding_info.border_width, bounding_info.boundary_min(1) - bounding_info.border_width},
            {bounding_info.boundary_min(0) - bounding_info.border_width, bounding_info.boundary_max(1) + bounding_info.border_width},
            {bounding_info.boundary_max(0) + bounding_info.border_width, bounding_info.boundary_max(1) + bounding_info.border_width},
            {bounding_info.boundary_max(0) + bounding_info.border_width, bounding_info.boundary_min(1) - bounding_info.border_width}};
        auto bc1 = subdivided_contour(bc0, 1.f, bounding_info.segment_length);
        compute_final_bounding_contour(bc1);
    } else {
        compute_final_bounding_contour(bounding_contour);
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
    std::list<RegionWithMargin<EntityType, std::list<std::list<FixedArray<CompressedScenePos, 2>>>>> hole_contours;
    CloseNeighborDetector<CompressedScenePos, 2> close_neighbor_detector{{(CompressedScenePos)10., (CompressedScenePos)10.}, 10};
    for (const auto& hole : hole_triangles) {
        // save_obj("/tmp/" + to_string(e) + ".obj", IndexedFaceSet<float, CompressedScenePos, size_t>{t}, nullptr);
        for (const auto& tt : hole.geometry) {
            for (const auto& v : tt.flat_iterable()) {
                if (close_neighbor_detector.contains_neighbor(
                    {v.position(0), v.position(1)},
                    (CompressedScenePos)1e-2,
                    DuplicateRule::NO_NEIGHBOR))
                {
                    if (!contour_triangles_filename.empty()) {
                        plot_tris(contour_triangles_filename + to_string(hole.hole_type) + ".current.obj", hole.geometry, { make_orderable(v.position) });
                        for (const auto& hole1 : hole_triangles) {
                            plot_tris(
                                contour_triangles_filename + to_string(hole1.hole_type) + ".other.obj",
                                hole1.geometry,
                                { make_orderable(v.position) });
                        }
                    }
                    auto exception = PointException<CompressedScenePos, 2>{
                        {v.position(0), v.position(1)},
                        "Detected near-duplicate point"};
                    THROW_OR_ABORT2(exception);
                }
            }
            {
                auto vt = FixedArray<CompressedScenePos, 3, 3>{
                    tt(0).position,
                    tt(1).position,
                    tt(2).position
                };
                auto tlc = triangle_largest_cosine(funpack(vt));
                if (std::isnan(tlc) || (tlc > BAD_TRIANGLE_COS)) {
                    auto exception = TriangleException<CompressedScenePos>{
                        vt[0], vt[1], vt[2],
                        "Detected bad triangle in hole \"" + to_string(hole.hole_type) + '"'};
                    THROW_OR_ABORT2(exception);
                }
            }
        }
        try {
            auto cs3 = find_contours(hole.geometry, contour_detection_strategy);
            auto& cs2 = hole_contours.emplace_back(hole.hole_type, hole.margin_type, hole.margin);
            for (const auto& c3 : cs3) {
                auto& c2 = cs2.geometry.emplace_back();
                for (const auto& p : c3) {
                    c2.push_back({p(0), p(1)});
                }
            }
            ncontours += cs2.geometry.size();
        } catch (const EdgeException<CompressedScenePos>& ex) {
            if (!contour_triangles_filename.empty()) {
                plot_tris(contour_triangles_filename, hole.geometry, { make_orderable(ex.a), make_orderable(ex.b) });
            }
            throw;
        }
    }
    std::vector<std::vector<p2t::Point*>> p2t_hole_contours;
    std::vector<EntityType> p2t_region_types;
    p2t_hole_contours.reserve(ncontours + 1);  // include bounding contour
    p2t_region_types.reserve(ncontours);

    p2t::CDT cdt{final_bounding_contour};
    auto add_contour = [&](EntityType region_type, const std::list<FixedArray<CompressedScenePos, 2>>& contour){
        p2t_region_types.push_back(region_type);
        auto& cnt = p2t_hole_contours.emplace_back();
        cnt.reserve(contour.size());
        // size_t i = 0;
        for (const auto& p : contour) {
            cnt.push_back(points(p));
            // draw_node(triangles, p.casted<float>(), 0.1 * float(i++) / c.size());
        }
        check_contour(cnt);
        cdt.AddHole(cnt);
    };
    auto add_margin_contour = [&](
        EntityType region_type,
        CompressedScenePos margin,
        const std::list<FixedArray<CompressedScenePos, 2>>& contour)
    {
        std::list<FixedArray<CompressedScenePos, 2>> m;
        add_contour(region_type, get_region_margin_contour(contour, margin, garden_margin));
    };
    for (auto& hc : hole_contours) {
        for (std::list<FixedArray<CompressedScenePos, 2>>& c : hc.geometry) {
            try {
                if (compute_area_ccw(c, scale) > 0) {
                    add_contour(hc.hole_type, c);
                    if (hc.margin != (CompressedScenePos)0.f) {
                        add_margin_contour(hc.margin_type, hc.margin, c);
                    }
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
    for (const auto& contour : region_contours) {
        try {
            add_contour(contour.hole_type, contour.geometry);
        } catch (const p2t::PointException& e) {
            throw p2t::PointException(e.point, "Could not add hole contour: " + std::string(e.what()));
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Could not add region contour: " + std::string(e.what()));
        }
    }
    for (auto* p : points.remaining_steiner_points()) {
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
            auto vt = FixedArray<CompressedScenePos, 3, 2>{*c0, *c1, *c2};
            {
                auto tlc = triangle_largest_cosine(funpack(vt));
                if (std::isnan(tlc) || (tlc > BAD_TRIANGLE_COS)) {
                    auto exception = TriangleException<CompressedScenePos>{
                        vt[0], vt[1], vt[2],
                        "Detected bad triangle"};
                    THROW_OR_ABORT2(exception);
                }
            }
            auto uv = terrain_uv<CompressedScenePos, double>(
                vt[0],
                vt[1],
                vt[2],
                scale,
                uv_scale,
                uv_period);
            tl->draw_triangle_wo_normals(
                {vt(0, 0), vt(0, 1), z},
                {vt(1, 0), vt(1, 1), z},
                {vt(2, 0), vt(2, 1), z},
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

}
