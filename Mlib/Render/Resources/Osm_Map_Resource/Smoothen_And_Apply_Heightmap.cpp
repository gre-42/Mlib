#include "Smoothen_And_Apply_Heightmap.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Normalized_Points_Fixed.hpp>
#include <Mlib/Images/PgmImage.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Apply_Height_Map.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Height_Binding.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Triangle_Lists.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Render/Resources/Resource_Instance_Descriptor.hpp>
#include <list>
#include <memory>

using namespace Mlib;

enum class SmoothingClass {
    RAISED,
    SMOOTHED
};

void Mlib::smoothen_and_apply_heightmap(
    const OsmResourceConfig& config,
    const std::map<OrderableFixedArray<float, 2>, HeightBinding>& height_bindings,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    const NormalizedPointsFixed& normalized_points,
    const std::list<std::shared_ptr<TriangleList>>& tls_buildings,
    const std::list<std::shared_ptr<TriangleList>>& tls_wall_barriers,
    const OsmTriangleLists& osm_triangle_lists,
    const OsmTriangleLists& air_triangle_lists,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    std::list<SteinerPointInfo>& steiner_points,
    std::list<StreetRectangle>& street_rectangles,
    std::map<WayPointLocation, std::list<std::pair<FixedArray<float, 3>, FixedArray<float, 3>>>>& way_point_edges_2_lanes)
{
    if (config.heightmap.empty() && config.street_edge_smoothness == 0 && config.terrain_edge_smoothness == 0) {
        return;
    }

    auto get_smoothed_vertices = [&](
        std::list<std::shared_ptr<TriangleList>>& tls_smoothed,
        std::list<FixedArray<float, 3>*>& smoothed_vertices,
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
        for (auto& d : object_resource_descriptors) {
            smoothed_vertices.push_back(&d.position);
        }
        for (auto& i : resource_instance_positions) {
            for (auto& d : i.second) {
                smoothed_vertices.push_back(&d.position);
            }
        }
        for (auto& h : hitboxes) {
            for (auto& d : h.second) {
                smoothed_vertices.push_back(&d);
            }
        }
        for (SteinerPointInfo& p : steiner_points) {
            smoothed_vertices.push_back(&p.position);
        }
        for (auto& r : street_rectangles) {
            for (auto& p : r.rectangle.flat_iterable()) {
                smoothed_vertices.push_back(&p);
            }
        }
        for (auto& w : way_point_edges_2_lanes) {
            for (auto& e : w.second) {
                smoothed_vertices.push_back(&e.first);
                smoothed_vertices.push_back(&e.second);
            }
        }
        {
            std::set<FixedArray<float, 3>*> svs(smoothed_vertices.begin(), smoothed_vertices.end());
            if (svs.size() != smoothed_vertices.size()) {
                throw std::runtime_error("Found duplicate smoothed vertices");
            }
        }
    };
    if (!config.heightmap.empty()) {
        LOG_INFO("apply_height_map");
        std::list<std::shared_ptr<TriangleList>> tls_smoothed;
        std::list<FixedArray<float, 3>*> smoothed_vertices;
        get_smoothed_vertices(tls_smoothed, smoothed_vertices, SmoothingClass::RAISED);
        std::set<const FixedArray<float, 3>*> vertices_to_delete;
        apply_height_map(
            *osm_triangle_lists.tl_terrain,
            osm_triangle_lists.entrances,
            config.default_tunnel_pipe_height,
            config.extrude_air_support_amount,
            smoothed_vertices,
            vertices_to_delete,
            PgmImage::load_from_file(config.heightmap).to_float() / 64.f * float(UINT16_MAX),
            normalized_points.chained(ScaleMode::DIAGONAL, OffsetMode::MINIMUM).normalization_matrix(),
            config.scale,
            nodes,
            ways,
            height_bindings,
            config.street_node_smoothness,
            config.layer_heights);
        for (auto& l : tls_smoothed) {
            l->triangles_.remove_if([&vertices_to_delete](const FixedArray<ColoredVertex, 3>& v){
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
        object_resource_descriptors.remove_if([&vertices_to_delete](const ObjectResourceDescriptor& d){
            return vertices_to_delete.contains(&d.position);
        });
        for (auto& i : resource_instance_positions) {
            i.second.remove_if([&vertices_to_delete](const ResourceInstanceDescriptor& d){
                return vertices_to_delete.contains(&d.position);
            });
        }
        for (auto& h : hitboxes) {
            h.second.remove_if([&vertices_to_delete](const FixedArray<float, 3>& p){
                return vertices_to_delete.contains(&p);
            });
        }
        steiner_points.remove_if([&vertices_to_delete](const SteinerPointInfo& p){
            return vertices_to_delete.contains(&p.position);});
        smoothed_vertices.remove_if([&vertices_to_delete](const FixedArray<float, 3>* p){
            return vertices_to_delete.contains(p);});
    }
    if (config.street_edge_smoothness > 0 || config.terrain_edge_smoothness > 0) {
        std::list<std::shared_ptr<TriangleList>> tls_smoothed;
        std::list<FixedArray<float, 3>*> smoothed_vertices;
        get_smoothed_vertices(tls_smoothed, smoothed_vertices, SmoothingClass::SMOOTHED);
        std::list<std::shared_ptr<TriangleList>> tls_street = osm_triangle_lists.tls_street();
        std::list<std::shared_ptr<TriangleList>> tls_air_street = air_triangle_lists.tls_street();
        tls_street.insert(tls_street.end(), tls_air_street.begin(), tls_air_street.end());
        if (config.street_edge_smoothness > 0) {
            LOG_INFO("smoothen_edges (street)");
            TriangleList::smoothen_edges(tls_street, {}, smoothed_vertices, config.street_edge_smoothness * config.scale, 100);
        }
        if (config.terrain_edge_smoothness > 0) {
            LOG_INFO("smoothen_edges (ground)");
            auto tls_smooth = osm_triangle_lists.tls_smooth();
            auto air_tls_smooth = air_triangle_lists.tls_smooth();
            tls_smooth.insert(tls_smooth.end(), air_tls_smooth.begin(), air_tls_smooth.end());
            TriangleList::smoothen_edges(tls_smooth, tls_street, smoothed_vertices, config.terrain_edge_smoothness * config.scale, 10);
        }
    }
}
