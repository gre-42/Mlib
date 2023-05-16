#include "Smoothen_And_Apply_Heightmap.hpp"
#include <Mlib/Geography/Heightmaps/Load_Heightmap_From_File.hpp>
#include <Mlib/Geometry/Coordinates/Normalized_Points_Fixed.hpp>
#include <Mlib/Geometry/Mesh/Point_Exception.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Images/PgmImage.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Apply_Heightmap.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Node_Height_Binding.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Triangle_Lists.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Rectangle.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Way_Point.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Vertex_Height_Binding.hpp>
#include <Mlib/Scene_Graph/Resources/Batch_Resource_Instantiator.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <list>
#include <memory>

using namespace Mlib;

enum class SmoothingClass {
    RAISED,
    SMOOTHED
};

void Mlib::smoothen_and_apply_heightmap(
    const OsmResourceConfig& config,
    const std::map<OrderableFixedArray<double, 2>, NodeHeightBinding>& node_height_bindings,
    std::map<const FixedArray<double, 3>*, VertexHeightBinding<double>>& vertex_height_bindings,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    const NormalizedPointsFixed<double>& normalized_points,
    const std::list<std::shared_ptr<TriangleList<double>>>& tls_buildings,
    const std::list<std::shared_ptr<TriangleList<double>>>& tls_wall_barriers,
    const OsmTriangleLists& osm_triangle_lists,
    const OsmTriangleLists& air_triangle_lists,
    VertexOutOfHeightMapBehavior vertex_out_of_height_map_behavior,
    BatchResourceInstantiator& bri,
    std::list<SteinerPointInfo>& steiner_points,
    std::list<FixedArray<double, 3>>& map_outer_contour3,
    std::list<StreetRectangle>& street_rectangles,
    std::map<WayPointLocation, std::list<std::pair<StreetWayPoint, StreetWayPoint>>>& way_point_edge_descriptors)
{
    LOG_FUNCTION("smoothen_and_apply_heightmap");
    if (config.heightmap.empty() && config.street_edge_smoothness == 0 && config.terrain_edge_smoothness == 0) {
        return;
    }

    auto get_smoothed_vertices = [&](
        std::list<std::shared_ptr<TriangleList<double>>>& tls_smoothed,
        std::list<FixedArray<double, 3>*>& smoothed_vertices,
        SmoothingClass sc)
    {
        auto tls_ground_smoothed =
            sc == SmoothingClass::RAISED
                ? osm_triangle_lists.tls_raised()
                : osm_triangle_lists.tls_smoothed();
        auto air_tls_smoothed =
            sc == SmoothingClass::RAISED
                ? air_triangle_lists.tls_raised()
                : air_triangle_lists.tls_smoothed();

        tls_smoothed.insert(tls_smoothed.end(), tls_ground_smoothed.begin(), tls_ground_smoothed.end());
        tls_smoothed.insert(tls_smoothed.end(), air_tls_smoothed.begin(), air_tls_smoothed.end());
        tls_smoothed.insert(tls_smoothed.end(), tls_buildings.begin(), tls_buildings.end());
        tls_smoothed.insert(tls_smoothed.end(), tls_wall_barriers.begin(), tls_wall_barriers.end());

        for (auto& l : tls_smoothed) {
            for (auto& t : l->triangles_) {
                for (auto& v : t.flat_iterable()) {
                    smoothed_vertices.push_back(&v.position);
                }
            }
        }
        bri.insert_into(smoothed_vertices);
        for (SteinerPointInfo& p : steiner_points) {
            smoothed_vertices.push_back(&p.position);
        }
        for (auto& p : map_outer_contour3) {
            smoothed_vertices.push_back(&p);
        }
        for (auto& r : street_rectangles) {
            for (auto& p : r.rectangle.flat_iterable()) {
                smoothed_vertices.push_back(&p);
            }
        }
        for (auto& w : way_point_edge_descriptors) {
            for (auto& e : w.second) {
                smoothed_vertices.push_back(&e.first.edge.first);
                smoothed_vertices.push_back(&e.first.edge.second);
                smoothed_vertices.push_back(&e.second.edge.first);
                smoothed_vertices.push_back(&e.second.edge.second);
            }
        }
        {
            std::set<FixedArray<double, 3>*> svs(smoothed_vertices.begin(), smoothed_vertices.end());
            if (svs.size() != smoothed_vertices.size()) {
                THROW_OR_ABORT("Found duplicate smoothed vertices");
            }
        }
    };
    if (!config.heightmap.empty()) {
        Array<double> heightmap = config.height_scale * load_heightmap_from_file<double>(config.heightmap);
        Array<bool> heightmap_mask;
        if (!config.heightmap_mask.empty()) {
            heightmap_mask = PgmImage::load_from_file(config.heightmap_mask).casted<bool>();
        }
        LOG_INFO("apply_heightmap");
        std::list<std::shared_ptr<TriangleList<double>>> tls_smoothed;
        std::list<FixedArray<double, 3>*> smoothed_vertices;
        get_smoothed_vertices(tls_smoothed, smoothed_vertices, SmoothingClass::RAISED);
        std::set<const FixedArray<double, 3>*> vertices_to_delete;
        apply_heightmap(
            *osm_triangle_lists.tl_terrain,
            osm_triangle_lists.entrances,
            config.default_tunnel_pipe_height,
            config.extrude_air_support_amount,
            smoothed_vertices,
            vertices_to_delete,
            heightmap,
            heightmap_mask,
            config.heightmap_extension,
            normalized_points.chained(ScaleMode::DIAGONAL, OffsetMode::MINIMUM).normalization_matrix(),
            config.scale,
            nodes,
            ways,
            node_height_bindings,
            vertex_height_bindings,
            config.street_node_smoothness,
            config.street_node_smoothing_iterations,
            config.layer_heights);
        if (!vertices_to_delete.empty()) {
            if (vertex_out_of_height_map_behavior == VertexOutOfHeightMapBehavior::THROW) {
                const FixedArray<double, 3>& v0 = **vertices_to_delete.begin();
                std::cerr << v0 << std::endl;
                throw PointException<double, 2>(FixedArray<double, 2>{ v0(0), v0(1) }, "One or more vertices out of heightmap range");
            } else if (vertex_out_of_height_map_behavior == VertexOutOfHeightMapBehavior::DELETE) {
                for (auto& l : tls_smoothed) {
                    l->triangles_.remove_if([&vertices_to_delete](const FixedArray<ColoredVertex<double>, 3>& v){
                        bool del =
                            vertices_to_delete.contains(&v(0).position) ||
                            vertices_to_delete.contains(&v(1).position) ||
                            vertices_to_delete.contains(&v(2).position);
                        if (del) {
                            vertices_to_delete.insert(&v(0).position);
                            vertices_to_delete.insert(&v(1).position);
                            vertices_to_delete.insert(&v(2).position);
                        }
                        return del;
                    });
                }
            } else {
                THROW_OR_ABORT("Unknown vertex out of heightmap behavior");
            }
        }
        bri.remove(vertices_to_delete);
        steiner_points.remove_if([&vertices_to_delete](const SteinerPointInfo& p){
            return vertices_to_delete.contains(&p.position);});
        smoothed_vertices.remove_if([&vertices_to_delete](const FixedArray<double, 3>* p){
            return vertices_to_delete.contains(p);});
    }
    if (config.street_edge_smoothness > 0 || config.terrain_edge_smoothness > 0) {
        std::list<std::shared_ptr<TriangleList<double>>> tls_smoothed;
        std::list<FixedArray<double, 3>*> smoothed_vertices;
        get_smoothed_vertices(tls_smoothed, smoothed_vertices, SmoothingClass::SMOOTHED);
        if (config.street_edge_smoothness > 0) {
            std::list<std::shared_ptr<TriangleList<double>>> tls_street = osm_triangle_lists.tls_street();
            std::list<std::shared_ptr<TriangleList<double>>> tls_air_street = air_triangle_lists.tls_street();
            tls_street.insert(tls_street.end(), tls_air_street.begin(), tls_air_street.end());
            LOG_INFO("smoothen_edges (street)");
            TriangleList<double>::smoothen_edges(vertex_height_bindings, tls_street, {}, smoothed_vertices, config.street_edge_smoothness * config.scale, 100, true);
        }
        if (config.terrain_edge_smoothness > 0) {
            LOG_INFO("smoothen_edges (ground)");

            auto tls_smooth = osm_triangle_lists.tls_smooth();
            auto air_tls_smooth = air_triangle_lists.tls_smooth();
            tls_smooth.insert(tls_smooth.end(), air_tls_smooth.begin(), air_tls_smooth.end());

            std::list<std::shared_ptr<TriangleList<double>>> tls_terrain_nosmooth = osm_triangle_lists.tls_terrain_nosmooth();
            std::list<std::shared_ptr<TriangleList<double>>> tls_air_terrain_nosmooth = air_triangle_lists.tls_terrain_nosmooth();
            tls_terrain_nosmooth.insert(tls_terrain_nosmooth.end(), tls_air_terrain_nosmooth.begin(), tls_air_terrain_nosmooth.end());

            TriangleList<double>::smoothen_edges(vertex_height_bindings, tls_smooth, tls_terrain_nosmooth, smoothed_vertices, config.terrain_edge_smoothness * config.scale, 100, false);
            // {
            //     std::list<FixedArray<ColoredVertex, 3>> tcp;
            //     for (const auto& l : tls_smooth) {
            //         for (const auto& t : l->triangles_) {
            //             tcp.push_back(t);
            //         }
            //     }
            //     save_obj("/tmp/tls_smooth.obj", IndexedFaceSet<float, size_t>{tcp});
            // }
            // {
            //     std::list<FixedArray<ColoredVertex, 3>> tcp;
            //     for (const auto& l : tls_street) {
            //         for (const auto& t : l->triangles_) {
            //             tcp.push_back(t);
            //         }
            //     }
            //     save_obj("/tmp/tls_street.obj", IndexedFaceSet<float, size_t>{tcp});
            // }
            // {
            //     std::list<FixedArray<ColoredVertex, 3>> tcp;
            //     for (const auto& l : tls_smoothed) {
            //         for (const auto& t : l->triangles_) {
            //             tcp.push_back(t);
            //         }
            //     }
            //     save_obj("/tmp/tls_smoothed.obj", IndexedFaceSet<float, size_t>{tcp});
            // }
        }
    }
}
