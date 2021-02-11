#include "Osm_Map_Resource.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Normalized_Points_Fixed.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Render/Renderables/Renderable_Osm_Map.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Calculate_Spawn_Points.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Calculate_Waypoints.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Draw_Streets.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Triangle_Lists.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Parse_Osm_Xml.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Smoothen_And_Apply_Heightmap.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Wayside_Resource_Names.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>

// #undef LOG_FUNCTION
// #undef LOG_INFO
// #define LOG_FUNCTION(msg) ::Mlib::Log log(msg, "LOG_OSM_MAP")
// #define LOG_INFO(msg) log.info(msg)

using namespace Mlib;

OsmMapResource::OsmMapResource(
    SceneNodeResources& scene_node_resources,
    const OsmResourceConfig& config)
: scene_node_resources_{ scene_node_resources },
  scale_{ config.scale },
  near_grass_resource_names_{ config.near_grass_resource_names },
  much_near_grass_distance_{ config.much_near_grass_distance }
{
    LOG_FUNCTION("OsmMapResource::OsmMapResource");
    std::map<std::string, Node> nodes;
    std::map<std::string, Way> ways;
    NormalizedPointsFixed normalized_points{ScaleMode::NONE, OffsetMode::CENTERED};

    parse_osm_xml(
        config.filename,
        config.scale,
        normalized_points,
        normalization_matrix_,
        nodes,
        ways);

    std::list<Building> buildings;
    std::list<Building> wall_barriers;
    if (config.with_buildings || config.with_roofs || config.with_ceilings) {
        buildings = get_buildings_or_wall_barriers(
            BuildingType::BUILDING,
            ways,
            config.building_bottom,
            config.default_building_top);
        compute_building_area(
            buildings,
            nodes,
            config.scale);
    }
    {
        wall_barriers = get_buildings_or_wall_barriers(
            BuildingType::WALL_BARRIER,
            ways,
            config.building_bottom,
            config.default_barrier_top);
    }
    {
        std::list<Building> spawn_lines = get_buildings_or_wall_barriers(
            BuildingType::SPAWN_LINE,
            ways,
            0,  // building_bottom
            0); // default_building_top
        for (const Building& bu : spawn_lines) {
            for (auto it = bu.way.nd.begin(); it != bu.way.nd.end(); ++it) {
                auto s = it;
                ++s;
                if (s != bu.way.nd.end()) {
                    FixedArray<float, 2> p = (nodes.at(*it).position + nodes.at(*s).position) / 2.f;
                    FixedArray<float, 2> dir = nodes.at(*it).position - nodes.at(*s).position;
                    float len2 = sum(squared(dir));
                    if (len2 < 1e-12) {
                        throw std::runtime_error("Spawn direction too small");
                    }
                    dir /= std::sqrt(len2);
                    spawn_points_.push_back(SpawnPoint{
                        .type = SpawnPointType::SPAWN_LINE,
                        .location = WayPointLocation::UNKNOWN,
                        .position = {p(0), p(1), bu.building_top * config.scale},
                        .rotation = {0.f, 0.f, std::atan2(dir(0), -dir(1))}});
                }
            }
        }
    }

    OsmTriangleLists osm_triangle_lists{config};
    tl_terrain_ = osm_triangle_lists.tl_terrain;
    std::list<std::shared_ptr<TriangleList>> tls_ground{
        osm_triangle_lists.tl_terrain,
        osm_triangle_lists.tl_terrain_visuals,
        osm_triangle_lists.tl_terrain_street_extrusion,
        osm_triangle_lists.tl_street_crossing,
        osm_triangle_lists.tl_path_crossing,
        osm_triangle_lists.tl_street,
        osm_triangle_lists.tl_path,
        osm_triangle_lists.tl_curb_street,
        osm_triangle_lists.tl_curb_path,
        osm_triangle_lists.tl_curb2_street,
        osm_triangle_lists.tl_curb2_path};
    std::list<std::shared_ptr<TriangleList>> tls_ground_wo_curb{
        osm_triangle_lists.tl_terrain,
        osm_triangle_lists.tl_terrain_visuals,
        osm_triangle_lists.tl_street_crossing,
        osm_triangle_lists.tl_path_crossing,
        osm_triangle_lists.tl_street,
        osm_triangle_lists.tl_path};
    std::list<std::shared_ptr<TriangleList>> tls_buildings;
    std::list<std::shared_ptr<TriangleList>> tls_wall_barriers;
    std::map<OrderableFixedArray<float, 2>, std::set<std::string>> height_bindings;
    std::list<SteinerPointInfo> steiner_points;
    std::list<StreetRectangle> street_rectangles;
    std::list<std::pair<std::string, std::string>> way_point_edges_1_lane;
    std::map<WayPointLocation, std::list<std::pair<FixedArray<float, 3>, FixedArray<float, 3>>>> way_point_edges_2_lanes;
    {
        ResourceNameCycle street_lights{scene_node_resources, config.street_light_resource_names};

        // draw_nodes(vertices, nodes, ways);
        // draw_test_lines(vertices, 0.02);
        // draw_ways(vertices, nodes, ways, 0.002);
        DrawStreets{DrawStreetsInput{
            *osm_triangle_lists.tl_street_crossing,
            *osm_triangle_lists.tl_path_crossing,
            *osm_triangle_lists.tl_street,
            *osm_triangle_lists.tl_path,
            *osm_triangle_lists.tl_curb_street,
            *osm_triangle_lists.tl_curb_path,
            *osm_triangle_lists.tl_curb2_street,
            *osm_triangle_lists.tl_curb2_path,
            resource_instance_positions_,
            object_resource_descriptors_,
            hitboxes_,
            street_rectangles,
            height_bindings,
            way_point_edges_1_lane,
            way_point_edges_2_lanes,
            nodes,
            ways,
            config.scale,
            config.uv_scale_street,
            config.default_street_width,
            config.only_raceways,
            config.highway_name_pattern,
            config.excluded_highways,
            config.path_tags,
            config.curb_alpha,
            config.curb2_alpha,
            config.curb_uv_x,
            config.curb2_uv_x,
            street_lights,
            config.with_height_bindings,
            config.driving_direction
        }};
    }

    if (config.forest_outline_tree_distance != INFINITY && !config.tree_resource_names.empty()) {
        ResourceNameCycle rnc{scene_node_resources, config.tree_resource_names};
        LOG_INFO("add_trees_to_forest_outlines");
        add_trees_to_forest_outlines(
            resource_instance_positions_,
            object_resource_descriptors_,
            hitboxes_,
            steiner_points,
            rnc,
            nodes,
            ways,
            config.forest_outline_tree_distance,
            config.forest_outline_tree_inwards_distance,
            config.scale);
        // add_binary_vegetation(
        //     tls,
        //     Material{
        //         texture: grass_texture,
        //         mixed_texture: "",
        //         overlap_npixels: 0,
        //         blend_mode: BlendMode::BINARY,
        //         wrap_mode: WrapMode::CLAMP_TO_EDGE,
        //         collide: false,
        //         aggregate_mode: AggregateMode::ONCE},
        //     grass_texture,
        //     tree_texture,
        //     tree_texture_2,
        //     *tl_terrain,
        //     scale);
    }
    // if (forest_outline_tree_distance != INFINITY) {
    //     add_grass_outlines(
    //         resource_instance_positions_,
    //         steiner_points,
    //         nodes,
    //         ways,
    //         continuous_vegetation,
    //         forest_outline_tree_distance / 3,
    //         forest_outline_tree_inwards_distance * 5,
    //         scale);
    // }
    if (config.raceway_beacon_distance != INFINITY) {
        LOG_INFO("add_beacons_to_raceways");
        add_beacons_to_raceways(
            object_resource_descriptors_,
            nodes,
            ways,
            config.raceway_beacon_distance,
            config.scale);
    }
    if (config.with_tree_nodes && !config.tree_resource_names.empty()) {
        ResourceNameCycle rnc{scene_node_resources, config.tree_resource_names};
        LOG_INFO("add_trees_to_tree_nodes");
        add_trees_to_tree_nodes(
            resource_instance_positions_,
            object_resource_descriptors_,
            hitboxes_,
            steiner_points,
            rnc,
            nodes,
            config.scale);
    }

    if (config.with_buildings) {
        LOG_INFO("draw_building_walls (facade)");
        draw_building_walls(
            tls_buildings,
            steiner_points,
            Material{
                .occluder_type = OccluderType::BLACK,
                .aggregate_mode = AggregateMode::ONCE,
                .ambience = {1.f, 1.f, 1.f},
                .specularity = {0.f, 0.f, 0.f},
                .draw_distance_noperations = 1000},
            buildings,
            nodes,
            config.scale,
            config.uv_scale_facade,
            config.max_wall_width,
            config.facade_textures);
    }
    {
        LOG_INFO("draw_building_walls (barrier)");
        draw_building_walls(
            tls_wall_barriers,
            steiner_points,
            Material{
                .occluder_type = OccluderType::BLACK,
                .blend_mode = config.barrier_blend_mode,
                .aggregate_mode = AggregateMode::ONCE,
                .is_small = false,
                .cull_faces = false,
                .draw_distance_noperations = 1000},
            wall_barriers,
            nodes,
            config.scale,
            config.uv_scale_barrier_wall,
            config.max_wall_width,
            { config.barrier_texture});
    }

    if (config.with_terrain) {
        std::vector<FixedArray<float, 2>> map_outer_contour;
        get_map_outer_contour(
            map_outer_contour,
            nodes,
            ways);

        auto hole_triangles = osm_triangle_lists.tl_street_crossing->triangles_;
        hole_triangles.insert(hole_triangles.end(), osm_triangle_lists.tl_path_crossing->triangles_.begin(), osm_triangle_lists.tl_path_crossing->triangles_.end());
        hole_triangles.insert(hole_triangles.end(), osm_triangle_lists.tl_street->triangles_.begin(), osm_triangle_lists.tl_street->triangles_.end());
        hole_triangles.insert(hole_triangles.end(), osm_triangle_lists.tl_path->triangles_.begin(), osm_triangle_lists.tl_path->triangles_.end());
        hole_triangles.insert(hole_triangles.end(), osm_triangle_lists.tl_curb_street->triangles_.begin(), osm_triangle_lists.tl_curb_street->triangles_.end());
        hole_triangles.insert(hole_triangles.end(), osm_triangle_lists.tl_curb_path->triangles_.begin(), osm_triangle_lists.tl_curb_path->triangles_.end());
        hole_triangles.insert(hole_triangles.end(), osm_triangle_lists.tl_curb2_street->triangles_.begin(), osm_triangle_lists.tl_curb2_street->triangles_.end());
        hole_triangles.insert(hole_triangles.end(), osm_triangle_lists.tl_curb2_path->triangles_.begin(), osm_triangle_lists.tl_curb2_path->triangles_.end());
        // plot_mesh(ArrayShape{2000, 2000}, tl_street->get_triangles_around({-1.59931f, 0.321109f}, 0.01f), {}, {{-1.59931f, 0.321109f, 0.f}}).save_to_file("/tmp/plt.pgm");
        // {
        //     std::list<FixedArray<ColoredVertex, 3>*> tf;
        //     for (auto& t : hole_triangles) {
        //         tf.push_back(&t);
        //     }
        //     plot_mesh_svg("/tmp/plt.svg", 800, 800, tf, {}, {});
        // }
        steiner_points = removed_duplicates(steiner_points, false);  // false = verbose
        BoundingInfo bounding_info{map_outer_contour, nodes, 0.1f};
        LOG_INFO("add_street_steiner_points");
        add_street_steiner_points(
            steiner_points,
            hole_triangles,
            bounding_info,
            config.scale,
            config.steiner_point_distances_road,
            config.steiner_point_distances_steiner);
        LOG_INFO("triangulate_terrain_or_ceilings");
        triangulate_terrain_or_ceilings(
            *tl_terrain_,
            osm_triangle_lists.tl_terrain_visuals.get(),
            config.blend_street
                ? std::list<std::list<FixedArray<ColoredVertex, 3>>>{osm_triangle_lists.tl_street->triangles_, osm_triangle_lists.tl_path->triangles_}
                : std::list<std::list<FixedArray<ColoredVertex, 3>>>{},
            bounding_info,
            steiner_points,
            map_outer_contour,
            hole_triangles,
            nodes,
            config.scale,
            config.uv_scale_terrain,
            0);
    }
    if (config.with_roofs) {
        LOG_INFO("draw_roofs");
        draw_roofs(
            tls_buildings,
            Material{
                .textures = {{.texture_descriptor = {.color = config.roof_texture}}},
                .occluder_type = OccluderType::BLACK,
                .aggregate_mode = AggregateMode::ONCE,
                .ambience = {1.f, 1.f, 1.f},
                .draw_distance_noperations = 1000}.compute_color_mode(),
            roof_color,
            buildings,
            nodes,
            config.roof_width,
            config.scale,
            roof_height0,
            roof_height1);
    }
    if (config.with_ceilings) {
        LOG_INFO("draw_ceilings");
        draw_ceilings(
            tls_buildings,
            Material{
                .textures = {{.texture_descriptor = {.color = config.ceiling_texture}}},
                .occluder_type = OccluderType::BLACK,
                .aggregate_mode = AggregateMode::ONCE,
                .ambience = {1.f, 1.f, 1.f},
                .specularity = {0.f, 0.f, 0.f},
                .draw_distance_noperations = 1000}.compute_color_mode(),
            buildings,
            nodes,
            config.scale,
            config.uv_scale_ceiling,
            config.max_wall_width);
    }
    if (config.remove_backfacing_triangles) {
        for (auto& l : tls_ground) {
            l->delete_backfacing_triangles();
        }
    }

    auto tls_all = smoothen_and_apply_heightmap(
        config,
        height_bindings,
        nodes,
        ways,
        normalized_points,
        tls_ground,
        tls_buildings,
        tls_wall_barriers,
        osm_triangle_lists,
        object_resource_descriptors_,
        resource_instance_positions_,
        hitboxes_,
        steiner_points,
        street_rectangles,
        way_point_edges_2_lanes);

    raise_streets(
        *osm_triangle_lists.tl_street_crossing,
        *osm_triangle_lists.tl_path_crossing,
        *osm_triangle_lists.tl_street,
        *osm_triangle_lists.tl_path,
        *osm_triangle_lists.tl_curb_street,
        *osm_triangle_lists.tl_curb_path,
        *osm_triangle_lists.tl_curb2_street,
        *osm_triangle_lists.tl_curb2_path,
        *osm_triangle_lists.tl_terrain,
        *osm_triangle_lists.tl_terrain_visuals,
        config.scale,
        config.raise_streets_amount);
    if (config.extrude_curb_amount != 0) {
        TriangleList::extrude(
            *osm_triangle_lists.tl_curb_street,
            {osm_triangle_lists.tl_curb_street},
            nullptr,
            config.extrude_curb_amount * config.scale,
            config.scale,
            config.uv_scale_street,
            config.uv_scale_street);
        TriangleList::extrude(
            *osm_triangle_lists.tl_curb_path,
            {osm_triangle_lists.tl_curb_path},
            nullptr,
            config.extrude_curb_amount * config.scale,
            config.scale,
            config.uv_scale_street,
            config.uv_scale_street);
    }
    if (config.extrude_street_amount != 0) {
        check_curb_validity(config.curb_alpha, config.curb2_alpha);
        if (config.curb_alpha == 1) {
            TriangleList::extrude(
                *osm_triangle_lists.tl_terrain_street_extrusion,
                {
                    osm_triangle_lists.tl_street,
                    osm_triangle_lists.tl_path,
                    osm_triangle_lists.tl_street_crossing,
                    osm_triangle_lists.tl_path_crossing},
                nullptr,
                config.extrude_street_amount * config.scale,
                config.scale,
                1,
                config.uv_scale_terrain);
        } else {
            // for (auto& t : tl_curb_street->triangles_) {
            //     for (auto& v : t.flat_iterable()) {
            //         v.uv(0) *= 0.5 * uv_scale_terrain * (curb2_alpha - curb_alpha) * default_street_width;
            //     }
            // }
            // for (auto& t : tl_curb_path->triangles_) {
            //     for (auto& v : t.flat_iterable()) {
            //         v.uv(0) *= 0.5 * uv_scale_terrain * (curb2_alpha - curb_alpha) * default_street_width;
            //     }
            // }
            // TriangleList::extrude(
            //     {tl_street, tl_path, tl_street_crossing, tl_path_crossing},
            //     extrude_street_amount * scale,
            //     scale,
            //     1,
            //     uv_scale_terrain,
            //     *tl_curb_street);
            std::list<std::shared_ptr<TriangleList>> source_vertices{
                osm_triangle_lists.tl_curb_street,
                osm_triangle_lists.tl_curb_path};
            TriangleList::extrude(
                *osm_triangle_lists.tl_curb_street,
                {
                    osm_triangle_lists.tl_street,
                    osm_triangle_lists.tl_path,
                    osm_triangle_lists.tl_street_crossing,
                    osm_triangle_lists.tl_path_crossing},
                &source_vertices,
                config.extrude_street_amount * config.scale,
                config.scale,
                1,
                config.uv_scale_terrain);
        }
    }
    // Normals are invalid after "apply_height_map"
    for (auto& l : tls_ground) {
        l->calculate_triangle_normals();
    }

    TriangleList::convert_triangle_to_vertex_normals(tls_wall_barriers);
    TriangleList::convert_triangle_to_vertex_normals(tls_ground_wo_curb);

    // for (auto& l : tls_ground) {
    //     colorize_height_map(l->triangles_);
    // }

    for (const WaysideResourceNames& ws : config.waysides) {
        LOG_INFO("add_grass_on_steiner_points");
        ResourceNameCycle rnc{scene_node_resources, ws.resource_names};
        add_grass_on_steiner_points(
            resource_instance_positions_,
            object_resource_descriptors_,
            hitboxes_,
            rnc,
            steiner_points,
            config.scale,
            ws.min_dist,
            ws.max_dist);
    }
    if (!config.grass_resource_names.empty()) {
        ResourceNameCycle rnc{scene_node_resources, config.grass_resource_names};
        LOG_INFO("add_grass_inside_triangles");
        add_grass_inside_triangles(
            resource_instance_positions_,
            object_resource_descriptors_,
            hitboxes_,
            rnc,
            *tl_terrain_,
            config.scale,
            config.much_grass_distance);
    }
    calculate_spawn_points(
        spawn_points_,
        street_rectangles,
        config.scale,
        config.driving_direction);
    {
        std::list<Building> way_point_lines = get_buildings_or_wall_barriers(
            BuildingType::WAYPOINTS,
            ways,
            0,  // building_bottom
            0); // default_building_top
        calculate_waypoints(
            way_points_[WayPointLocation::STREET],
            way_point_lines,
            way_point_edges_1_lane,
            way_point_edges_2_lanes[WayPointLocation::STREET],
            nodes);
        calculate_waypoints(
            way_points_[WayPointLocation::SIDEWALK],
            way_point_lines,
            {},
            way_point_edges_2_lanes[WayPointLocation::SIDEWALK],
            nodes);
    }
    // way_points_.at(WayPointLocation::STREET).plot("/tmp/way_points_street.svg", 600, 600, 0.1);
    // way_points_.at(WayPointLocation::SIDEWALK).plot("/tmp/way_points_sidewalk.svg", 600, 600, 0.1);

    for (auto& l : tls_all) {
        if (!l->triangles_.empty()) {
            cvas_.push_back(l->triangle_array());
        }
    }
}

void OsmMapResource::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const
{
    {
        size_t i = 0;
        for (auto& p : object_resource_descriptors_) {
            auto node = new SceneNode;
            node->set_position(p.position);
            node->set_scale(scale_ * p.scale);
            node->set_rotation({float{M_PI} / 2.f, 0.f, 0.f});
            scene_node_resources_.instantiate_renderable(p.name, p.name, *node, resource_filter);
            if (node->requires_render_pass()) {
                scene_node.add_child(p.name + "-" + std::to_string(i++), node);
            } else {
                std::cerr << "Adding aggregate " << p.name << std::endl;
                scene_node.add_aggregate_child(p.name + "-" + std::to_string(i++), node);
            }
        }
    }
    for (auto& p : resource_instance_positions_) {
        auto node = new SceneNode;
        node->set_rotation({ float{M_PI} / 2.f, 0.f, 0.f });
        scene_node_resources_.instantiate_renderable(p.first, p.first, *node, resource_filter);
        if (node->requires_render_pass()) {
            throw std::runtime_error("Object " + p.first + " requires render pass");
        }
        scene_node.add_instances_child(p.first, node);
        for (const auto& r : p.second) {
            scene_node.add_instances_position(p.first, r.position);
        }
    }
    if (!near_grass_resource_names_.empty() && much_near_grass_distance_ != INFINITY) {
        scene_node.add_renderable("osm_map_near", std::make_shared<RenderableOsmMap>(this));
    }
    // if (rbvh_ == nullptr) {
    //     rbvh_ = std::make_shared<BvhResource>(cvas_);
    // }
    // rbvh_->instantiate_renderable(name, scene_node, resource_filter);
    if (rcva_ == nullptr) {
        rcva_ = std::make_shared<ColoredVertexArrayResource>(cvas_, nullptr);
    }
    rcva_->instantiate_renderable(name, scene_node, resource_filter);
}

std::shared_ptr<AnimatedColoredVertexArrays> OsmMapResource::get_animated_arrays() const {
    auto res = std::make_shared<AnimatedColoredVertexArrays>();
    res->cvas = cvas_;
    // Append scaled hitboxes
    for (auto& p : hitboxes_) {
        for (auto& x : scene_node_resources_.get_animated_arrays(p.first)->cvas) {
            for (auto& y : p.second) {
                res->cvas.push_back(x->transformed(
                    TransformationMatrix{
                        scale_ * fixed_identity_array<float, 3>(),
                        y}));
            }
        }
    }
    return res;
}

TransformationMatrix<double, 3> OsmMapResource::get_geographic_mapping(SceneNode& scene_node) const
{
    TransformationMatrix<double, 3> m3;
    const auto& R2 = normalization_matrix_.R();
    const auto& t2 = normalization_matrix_.t();
    m3.R() = FixedArray<double, 3, 3>{
        R2(0, 0), R2(0, 1), 0,
        R2(1, 0), R2(1, 1), 0,
        0, 0, scale_};
    m3.t() = FixedArray<double, 3>{
        t2(0),
        t2(1),
        scale_};
    return TransformationMatrix<double, 3>{inv((scene_node.absolute_model_matrix().casted<double>() * m3).affine())};
}

std::list<SpawnPoint> OsmMapResource::spawn_points() const {
    return spawn_points_;
}

std::map<WayPointLocation, PointsAndAdjacency<float, 2>> OsmMapResource::way_points() const {
    return way_points_;
}
