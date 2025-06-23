#include "Apply_Heightmap_And_Smoothen.hpp"
#include <Mlib/Geography/Heightmaps/Load_Heightmap_From_File.hpp>
#include <Mlib/Geometry/Coordinates/Normalized_Points_Fixed.hpp>
#include <Mlib/Geometry/Exceptions/Point_Exception.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Images/Filters/Maximum_Filter.hpp>
#include <Mlib/Images/Pgm_Image.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Apply_Heightmap.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Height_Sampler.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Node_Height_Binding.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Triangle_Lists.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Rectangle.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Way_Point.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Vertex_Height_Binding.hpp>
#include <Mlib/Scene_Graph/Resources/Batch_Resource_Instantiator.hpp>
#include <Mlib/Threads/Thread_Top.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <list>
#include <memory>

using namespace Mlib;

enum class SmoothingClass {
    RAISED,
    SMOOTHED
};

void Mlib::apply_heightmap_and_smoothen(
    const OsmResourceConfig& config,
    const StreetBvh& ground_street_bvh,
    const StreetBvh& air_bvh,
    const std::map<OrderableFixedArray<CompressedScenePos, 2>, NodeHeightBinding>& node_height_bindings,
    std::unordered_map<FixedArray<CompressedScenePos, 3>*, VertexHeightBinding<CompressedScenePos>>& vertex_height_bindings,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    const NormalizedPointsFixed<ScenePos>& normalized_points,
    const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls_wall_barriers,
    const OsmTriangleLists& osm_triangle_lists,
    const OsmTriangleLists& air_triangle_lists,
    VertexOutOfHeightMapBehavior vertex_out_of_height_map_behavior,
    BatchResourceInstantiator& bri,
    std::list<SteinerPointInfo>& steiner_points,
    std::list<FixedArray<CompressedScenePos, 3>>& map_outer_contour3,
    std::list<StreetRectangle>& street_rectangles,
    std::map<WayPointSandbox, std::list<std::pair<StreetWayPoint, StreetWayPoint>>>& way_point_edge_descriptors)
{
    FunctionGuard fg{ "Apply heightmap and smoothen" };
    if (config.heightmap.empty() &&
        (config.street_edge_smoothness == 0) &&
        (config.terrain_edge_smoothness == 0))
    {
        return;
    }

    std::map<CompressedScenePos*, CompressedScenePos> psharp_heights;
    {
        std::map<OrderableFixedArray<CompressedScenePos, 2>, CompressedScenePos> sharp_heights;
        for (const auto& [_, w] : ways) {
            auto it = w.tags.find("sharp_height");
            if (it == w.tags.end()) {
                continue;
            }
            auto sharp_height = (CompressedScenePos)safe_stod(it->second);
            for (const auto& n : w.nd) {
                const auto& pos = nodes.at(n).position;
                sharp_heights.try_emplace(OrderableFixedArray(pos), sharp_height);
            }
        }
        if (!sharp_heights.empty()) {
            for (auto& l : osm_triangle_lists.tls_smooth()) {
                for (auto& t : l->triangles) {
                    for (auto& v : t.flat_iterable()) {
                        auto it = sharp_heights.find(OrderableFixedArray<CompressedScenePos, 2>{
                            v.position(0), v.position(1)});
                        if (it != sharp_heights.end()) {
                            psharp_heights[&v.position(2)] = it->second;
                        }
                    }
                }
            }
        }
    }

    auto get_smoothed_vertices = [&](
        std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls_smoothed,
        std::list<FixedArray<CompressedScenePos, 3>*>& smoothed_vertices,
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
        // tls_smoothed.insert(tls_smoothed.end(), tls_buildings.begin(), tls_buildings.end());
        tls_smoothed.insert(tls_smoothed.end(), tls_wall_barriers.begin(), tls_wall_barriers.end());

        for (auto& l : tls_smoothed) {
            for (auto& t : l->triangles) {
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
            for (auto& p0 : r.rectangle.row_iterable()) {
                for (auto& p : p0.row_iterable()) {
                    smoothed_vertices.push_back(&p);
                }
            }
        }
        for (auto& [_, w] : way_point_edge_descriptors) {
            for (auto& [p0, p1] : w) {
                smoothed_vertices.push_back(&p0.edge.first);
                smoothed_vertices.push_back(&p0.edge.second);
                smoothed_vertices.push_back(&p1.edge.first);
                smoothed_vertices.push_back(&p1.edge.second);
            }
        }
        {
            std::set<FixedArray<CompressedScenePos, 3>*> svs(smoothed_vertices.begin(), smoothed_vertices.end());
            if (svs.size() != smoothed_vertices.size()) {
                THROW_OR_ABORT("Found duplicate smoothed vertices");
            }
        }
    };
    std::optional<HeightSampler> height_sampler;
    if (!config.heightmap.empty()) {
        Array<double> heightmap = config.height_scale * load_heightmap_from_file<double>(config.heightmap);
        if (config.heightmap_dilation != 0) {
            heightmap = maximum_filter(heightmap, config.heightmap_dilation);
        }
        Array<bool> heightmap_mask;
        if (!config.heightmap_mask.empty()) {
            heightmap_mask = PgmImage::load_from_file(config.heightmap_mask).casted<bool>();
        } else {
            heightmap_mask.move() = ones<bool>(heightmap.shape());
        }
        size_t ext = std::max({ heightmap.shape(0), heightmap.shape(1), config.heightmap_extension });
        ExtendedImage extended_heightmap{
            heightmap,
            heightmap_mask,
            config.heightmap_extension,
            50,
            1 + ext / 50 };
        height_sampler.emplace(
            std::move(extended_heightmap),
            normalized_points.chained(ScaleMode::DIAGONAL, OffsetMode::MINIMUM).normalization_matrix());

        LOG_INFO("apply_heightmap");
        std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> tls_smoothed;
        std::list<FixedArray<CompressedScenePos, 3>*> smoothed_vertices;
        get_smoothed_vertices(tls_smoothed, smoothed_vertices, SmoothingClass::RAISED);
        std::set<const FixedArray<CompressedScenePos, 3>*> vertices_to_delete;
        fg.update("Applying heightmap");
        apply_heightmap(
            *osm_triangle_lists.tl_terrain,
            osm_triangle_lists.entrances,
            config.default_tunnel_pipe_height,
            config.extrude_air_support_amount,
            smoothed_vertices,
            vertices_to_delete,
            *height_sampler,
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
                const FixedArray<CompressedScenePos, 3>& v0 = **vertices_to_delete.begin();
                lerr() << v0;
                throw PointException<CompressedScenePos, 2>(FixedArray<CompressedScenePos, 2>{ v0(0), v0(1) }, "One or more vertices out of heightmap range");
            } else if (vertex_out_of_height_map_behavior == VertexOutOfHeightMapBehavior::DELETE) {
                for (auto& l : tls_smoothed) {
                    l->triangles.remove_if([&vertices_to_delete](const FixedArray<ColoredVertex<CompressedScenePos>, 3>& v){
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
        smoothed_vertices.remove_if([&vertices_to_delete](const FixedArray<CompressedScenePos, 3>* p){
            return vertices_to_delete.contains(p);});
    }
    if (config.street_edge_smoothness > 0 || config.terrain_edge_smoothness > 0) {
        std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> tls_smoothed;
        std::list<FixedArray<CompressedScenePos, 3>*> smoothed_vertices;
        get_smoothed_vertices(tls_smoothed, smoothed_vertices, SmoothingClass::SMOOTHED);
        auto tls_move_only_z = osm_triangle_lists.tls_terrain_smooth_only_z();
        auto tls_air_move_only_z = air_triangle_lists.tls_terrain_smooth_only_z();
        tls_move_only_z.insert(tls_move_only_z.end(), tls_air_move_only_z.begin(), tls_air_move_only_z.end());
        if (config.street_edge_smoothness > 0) {
            std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> tls_street = osm_triangle_lists.tls_street();
            std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> tls_air_street = air_triangle_lists.tls_street();
            tls_street.insert(tls_street.end(), tls_air_street.begin(), tls_air_street.end());
            fg.update("Smoothen edges (street)");
            TriangleList<CompressedScenePos>::smoothen_edges(
                vertex_height_bindings,                             // vertex_height_bindings
                {},                                                 // bias
                tls_street,                                         // edge_triangle_lists
                tls_move_only_z,                                    // excluded_triangle_lists
                {},                                                 // move_only_z_triangle_lists
                smoothed_vertices,                                  // smoothed_vertices
                config.street_edge_smoothness* config.scale,        // smoothness
                100,                                                // niterations
                true);                                              // move_only_z
        }
        if (config.terrain_edge_smoothness > 0) {
            fg.update("Smoothen edges (ground)");

            auto tls_smooth = osm_triangle_lists.tls_smooth();
            auto air_tls_smooth = air_triangle_lists.tls_smooth();
            tls_smooth.insert(tls_smooth.end(), air_tls_smooth.begin(), air_tls_smooth.end());

            std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> tls_terrain_nosmooth = osm_triangle_lists.tls_terrain_nosmooth();
            std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> tls_air_terrain_nosmooth = air_triangle_lists.tls_terrain_nosmooth();
            tls_terrain_nosmooth.insert(tls_terrain_nosmooth.end(), tls_air_terrain_nosmooth.begin(), tls_air_terrain_nosmooth.end());

            std::unordered_map<OrderableFixedArray<CompressedScenePos, 3>, FixedArray<float, 3>> bias;
            if (height_sampler.has_value() && !config.terrain_edge_bias.empty()) {
                for (const auto* s : smoothed_vertices) {
                    FixedArray<CompressedScenePos, 2> pt{ (*s)(0), (*s)(1) };
                    FixedArray<CompressedScenePos, 2> closest_ground = uninitialized;
                    FixedArray<CompressedScenePos, 2> closest_air = uninitialized;
                    auto teb = (CompressedScenePos)(config.terrain_edge_bias.xmax() * config.scale);
                    auto ground_dist = ground_street_bvh.min_dist(pt, teb, &closest_ground).value_or(teb);
                    auto air_dist = air_bvh.min_dist(pt, teb, &closest_air).value_or(teb);
                    if ((ground_dist < teb) || (air_dist < teb)) {
                        auto closest_dist = std::min(ground_dist, air_dist);
                        auto closest_pt = (ground_dist < air_dist)
                            ? closest_ground
                            : closest_air;
                        CompressedScenePos street_z;
                        if ((*height_sampler)(closest_pt, street_z)) {
                            bias.try_emplace(
                                OrderableFixedArray(*s),
                                FixedArray<CompressedScenePos, 3>{
                                    (CompressedScenePos)0.,
                                    (CompressedScenePos)0.,
                                    (CompressedScenePos)config.terrain_edge_bias(
                                        (float)(closest_dist * sign(funpack((*s)(2)) / config.scale - funpack(street_z)))) });
                        }
                    }
                }
            }
            TriangleList<CompressedScenePos>::smoothen_edges(
                vertex_height_bindings,                             // vertex_height_bindings
                bias,                                               // bias
                tls_smooth,                                         // edge_triangle_lists
                tls_terrain_nosmooth,                               // excluded_triangle_lists
                tls_move_only_z,                                    // move_only_z_triangle_lists
                smoothed_vertices,                                  // smoothed_vertices
                config.terrain_edge_smoothness * config.scale,      // smoothness
                100,                                                // niterations
                false);                                             // move_only_z
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
    for (auto& [p, h] : psharp_heights) {
        *p = h;
    }
}
