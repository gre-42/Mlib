#include "Triangulate_Water.hpp"
#include <Mlib/Geometry/Coordinates/Barycentric_Coordinates.hpp>
#include <Mlib/Geometry/Intersection/Intersectable_Point.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Close_Neighbor_Detector.hpp>
#include <Mlib/Geometry/Mesh/Contour_Detection_Strategy.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Height_Contours.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Math/Fixed_Rotation_2D.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Compute_Area.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Triangulate_Entity_List.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Water_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Way_Bvh.hpp>
#include <Mlib/Render/Resources/Heterogeneous_Resource.hpp>

using namespace Mlib;

class HoleIndicator {
    using Triangle2d = FixedArray<CompressedScenePos, 3, 2>;
public:
    explicit HoleIndicator(
        const std::list<RegionWithMargin<WaterType, std::list<FixedArray<CompressedScenePos, 2>>>>& contours)
        : bvh_{ {(CompressedScenePos)100.f, (CompressedScenePos)100.f}, 10 }
    {
        for (const auto& contour : contours) {
            if (contour.hole_type != WaterType::STEEP_HOLE) {
                THROW_OR_ABORT("HoleIndicator: Unexpected hole type");
            }
            BoundingInfo bounding_info{
                std::vector(contour.geometry.begin(), contour.geometry.end()),
                {},
                (CompressedScenePos)5.f,
                (CompressedScenePos)2.f };
            WaterTypeTriangleList tl_holes;
            auto tl_steep_holes = std::make_shared<TriangleList<CompressedScenePos>>(
                "steep_holes",
                Material{},
                Morphology{ PhysicsMaterial::NONE });
            auto tl_undefined = std::make_shared<TriangleList<CompressedScenePos>>(
                "undefined",
                Material{},
                Morphology{ PhysicsMaterial::NONE });
            tl_holes.insert(WaterType::STEEP_HOLE, tl_steep_holes);
            tl_holes.insert(WaterType::UNDEFINED, tl_undefined);
            triangulate_entity_list(
                tl_holes,                                       // tl_terrain
                bounding_info,                                  // bounding_info
                {},                                             // steiner_points
                {},                                             // bounding_contour
                {},                                             // hole_triangles
                {{WaterType::STEEP_HOLE, WaterType::UNDEFINED,  // region_contours
                    (CompressedScenePos)0.f, contour.geometry}},
                1.f,                                            // scale
                1.f,                                            // triangulation_scale
                1.f,                                            // uv_scale
                1.f,                                            // uv_period
                (CompressedScenePos)0.f,                        // z
                fixed_zeros<float, 3>(),                        // color
                "",                                             // contour_triangles_filename
                "",                                             // contour_filename
                "",                                             // triangle_filename
                WaterType::UNDEFINED,                           // bounding_terrain_type
                WaterType::UNDEFINED,                           // default_terrain_type
                {},                                             // excluded_terrain_types
                ContourDetectionStrategy::EDGE_NEIGHBOR,
                {});                                            // garden_margin
            for (const auto& t : tl_holes[WaterType::STEEP_HOLE]->triangles) {
                auto t2 = Triangle2d{
                    FixedArray<CompressedScenePos, 2>{t(0).position(0), t(0).position(1)},
                    FixedArray<CompressedScenePos, 2>{t(1).position(0), t(1).position(1)},
                    FixedArray<CompressedScenePos, 2>{t(2).position(0), t(2).position(1)}};
                bvh_.insert(AxisAlignedBoundingBox<CompressedScenePos, 2>::from_points(t2), t2);
            }
        }
    }
    bool is_in_hole(const FixedArray<CompressedScenePos, 2>& pt) const {
        return !bvh_.visit(
            IntersectablePoint{ pt },
            [&pt](const Triangle2d& tri2){
                FixedArray<double, 3> coords = uninitialized;
                barycentric(
                    funpack(pt),
                    funpack(tri2[0]),
                    funpack(tri2[1]),
                    funpack(tri2[2]),
                    coords(0),
                    coords(1),
                    coords(2));
                if (all(coords >= double{ -1e-3 }) && all(coords <= 1.f + double{ 1e-3 })) {
                    return false;
                }
                return true;
            });
    }
    bool is_in_hole(const FixedArray<ColoredVertex<CompressedScenePos>, 3>& t) const {
        for (const auto& v : t.flat_iterable()) {
            if (!is_in_hole(FixedArray<CompressedScenePos, 2>{v.position(0), v.position(1)})) {
                return false;
            }
        }
        return true;
    }
private:
    Bvh<CompressedScenePos, 2, Triangle2d> bvh_;
};

void Mlib::find_coast_contours(
    std::list<RegionWithMargin<WaterType, std::list<FixedArray<CompressedScenePos, 2>>>>& contours,
    const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& terrain,
    const FixedArray<CompressedScenePos, 2>& water_heights)
{
    HoleIndicator hi{ contours };
    if (water_heights(0) != water_heights(1)) {
        HeightContoursByVertex hc{ water_heights(0) };
        for (const auto& ts : terrain) {
            for (const auto& t : ts->triangles) {
                if (!hi.is_in_hole(t)) {
                    hc.add_triangle({t(0).position, t(1).position, t(2).position});
                }
            }
        }
        for (auto& c : hc.get_contours_and_clear()) {
            if (compute_area_ccw(c, 1.) > 0) {
                contours.emplace_back(WaterType::UNDEFINED, WaterType::UNDEFINED, (CompressedScenePos)0.f, c);
            } else {
                c.reverse();
                contours.emplace_back(WaterType::UNDEFINED, WaterType::UNDEFINED, (CompressedScenePos)0.f, c);
            }
        }
    }
    {
        HeightContoursByVertex hc{ water_heights(1) };
        for (const auto& ts : terrain) {
            for (const auto& t : ts->triangles) {
                if (!hi.is_in_hole(t)) {
                    hc.add_triangle({t(0).position, t(1).position, t(2).position});
                }
            }
        }
        for (auto& c : hc.get_contours_and_clear()) {
            if (compute_area_ccw(c, 1.) > 0) {
                contours.emplace_back(WaterType::SHALLOW_HOLE, WaterType::UNDEFINED, (CompressedScenePos)0.f, c);
            } else {
                c.reverse();
                contours.emplace_back(WaterType::SHALLOW_LAKE, WaterType::UNDEFINED, (CompressedScenePos)0.f, c);
            }
        }
    }
}

void Mlib::add_water_steiner_points(
    std::list<SteinerPointInfo>& steiner_points,
    const std::list<RegionWithMargin<WaterType, std::list<FixedArray<CompressedScenePos, 2>>>>& contours,
    const AxisAlignedBoundingBox<CompressedScenePos, 2>& bounds,
    const FixedArray<CompressedScenePos, 2>& cell_size,
    float yangle,
    CompressedScenePos duplicate_distance,
    CompressedScenePos water_height)
{
    CloseNeighborDetector<CompressedScenePos, 2> close_neighbor_detector{{(CompressedScenePos)10., (CompressedScenePos)10.}, 10};
    for (const auto& s : steiner_points) {
        if (close_neighbor_detector.contains_neighbor(
            {s.position(0), s.position(1)},
            duplicate_distance,
            DuplicateRule::IS_NEIGHBOR))
        {
            THROW_OR_ABORT("Unexpected duplicate in steiner points");
        }
    }
    for (const auto& c : contours) {
        for (const auto& p : c.geometry) {
            close_neighbor_detector.contains_neighbor(
                p,
                duplicate_distance,
                DuplicateRule::IS_NEIGHBOR);
        }
    }
    {
        auto R = fixed_rotation_2d<ScenePos>(yangle);
        FixedArray<CompressedScenePos, 2> xy = uninitialized;
        for (xy(0) = bounds.min(0); xy(0) < bounds.max(0); xy(0) += cell_size(0)) {
            for (xy(1) = bounds.min(1); xy(1) < bounds.max(1); xy(1) += cell_size(1)) {
                auto rxy = dot1d(R, funpack(xy)).casted<CompressedScenePos>();
                if (bounds.contains(rxy) &&
                    !close_neighbor_detector.contains_neighbor(
                        rxy,
                        duplicate_distance,
                        DuplicateRule::IS_NEIGHBOR))
                {
                    steiner_points.emplace_back(
                        FixedArray<CompressedScenePos, 3>{rxy(0), rxy(1), water_height},
                        SteinerPointType::WATER);
                }
            }
        }
    }
}

void Mlib::triangulate_water(
    WaterTypeTriangleList& tl_water,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const std::vector<FixedArray<CompressedScenePos, 2>>& bounding_contour,
    const std::list<RegionWithMargin<WaterType, std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>>>& hole_triangles,
    const std::list<RegionWithMargin<WaterType, std::list<FixedArray<CompressedScenePos, 2>>>>& region_contours,
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
        { WaterType::SHALLOW_HOLE, WaterType::STEEP_HOLE }, // excluded_entitities
        contour_detection_strategy,
        {});                                                // garden_margin
}

void Mlib::set_water_alpha(
    WaterTypeTriangleList& tl_water,
    const std::list<RegionWithMargin<WaterType, std::list<FixedArray<CompressedScenePos, 2>>>>& region_contours,
    CompressedScenePos max_dist,
    CompressedScenePos coast_width)
{
    ScenePos steepness = 1. / (ScenePos)coast_width;
    WayBvh way_bvh;
    for (const auto& c : region_contours) {
        if (any(c.hole_type & WaterType::ANY_SHALLOW)) {
            way_bvh.add_path(c.geometry);
        }
    }
    for (auto& [_, x] : tl_water.map()) {
        if (!x->alpha.empty()) {
            THROW_OR_ABORT("Water already has alpha values");
        }
        for (auto& t : x->triangles) {
            auto& a = x->alpha.emplace_back(uninitialized);
            for (size_t i = 0; i < 3; ++i) {
                FixedArray<ScenePos, 2> dir = uninitialized;
                CompressedScenePos distance;
                if (way_bvh.nearest_way(
                    {t(i).position(0), t(i).position(1)},
                    max_dist,
                    dir,
                    distance))
                {
                    a(i) = (float)(steepness * (ScenePos)distance);
                } else {
                    a(i) = 1.f;
                }
            }
        }
    }
}
