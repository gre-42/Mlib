#include "Osm_Map_Resource.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Edge_Exception.hpp>
#include <Mlib/Geometry/Mesh/Mesh_Subtract.hpp>
#include <Mlib/Geometry/Mesh/Plot.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Geometry/Mesh/Save_Obj.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Mesh/Triangles_Around.cpp>
#include <Mlib/Geometry/Normalized_Points_Fixed.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Render/Renderables/Renderable_Osm_Map.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Add_Street_Steiner_Points.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Bounding_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Calculate_Spawn_Points.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Calculate_Waypoint_Adjacency.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Delete_Backfacing_Triangles.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Draw_Streets.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Entrance_Type.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Get_Buildings_Or_Wall_Barriers.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Get_Map_Outer_Contour.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Get_Terrain_Region_Contours.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Get_Water_Region_Contours.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Height_Binding.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Triangle_Lists.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Parse_Osm_Xml.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Report_Osm_Problems.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Road_Type.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Smoothen_And_Apply_Heightmap.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Styled_Road.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Terrain_Type.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Triangulate_Terrain_Or_Ceilings.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Water_Type.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Wayside_Resource_Names.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <poly2tri/point_exception.hpp>

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
    
    auto handle_point_exception = [this](const p2t::PointException& e, const std::string& message) {
        FixedArray<double, 3> pos{e.point.x, e.point.y, 0.};
        auto m = get_geographic_mapping(SceneNode());
        std::stringstream sstr;
        sstr.precision(15);
        sstr << message << " at position " << m.transform(pos) << ": " << e.what() << std::endl;
        throw std::runtime_error(sstr.str());
    };
    auto handle_edge_exception = [this](const EdgeException& e, const std::string& message){
        auto m = get_geographic_mapping(SceneNode());
        std::stringstream sstr;
        sstr.precision(15);
        sstr << message << " at edge " <<
            e.a <<
            " -> " <<
            e.b <<
            " | " <<
            m.transform(e.a.casted<double>()) <<
            " -> " <<
            m.transform(e.b.casted<double>()) <<
            ": " << e.what() << std::endl;
        throw std::runtime_error(sstr.str());
    };
    auto handle_triangle_exception = [this](const TriangleException& e, const std::string& message){
        auto m = get_geographic_mapping(SceneNode());
        std::stringstream sstr;
        sstr.precision(15);
        sstr << message << " at triangle " <<
            e.a <<
            " <-> " <<
            e.b <<
            " <-> " <<
            e.c <<
            " | " <<
            m.transform(e.a.casted<double>()) <<
            " <-> " <<
            m.transform(e.b.casted<double>()) <<
            " <-> " <<
            m.transform(e.c.casted<double>()) <<
            ": " << e.what() << std::endl;
        throw std::runtime_error(sstr.str());
    };

    report_osm_problems(nodes, ways);

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

    std::list<std::pair<TerrainType, std::list<FixedArray<float, 3>>>> terrain_region_contours =
        get_terrain_region_contours(nodes, ways);

    auto& tunnel_pipe_cvas = scene_node_resources.get_animated_arrays(config.tunnel_pipe_resource_name)->cvas;
    if (tunnel_pipe_cvas.size() != 1) {
        throw std::runtime_error("Pipe does not have exactly one mesh");
    }
    auto& tunnel_pipe_cva = tunnel_pipe_cvas.front();
    auto& tunnel_bdry_cvas = scene_node_resources.get_animated_arrays(config.tunnel_bdry_resource_name)->cvas;
    if (tunnel_bdry_cvas.size() != 1) {
        throw std::runtime_error("bdry does not have exactly one mesh");
    }
    auto& tunnel_bdry_cva = tunnel_bdry_cvas.front();

    OsmTriangleLists osm_triangle_lists{config};
    OsmTriangleLists air_triangle_lists{config};
    tl_terrain_ = osm_triangle_lists.tl_terrain;
    std::list<std::shared_ptr<TriangleList>> tls_buildings;
    std::list<std::shared_ptr<TriangleList>> tls_wall_barriers;
    std::map<OrderableFixedArray<float, 2>, HeightBinding> height_bindings;
    std::list<SteinerPointInfo> steiner_points;
    std::list<StreetRectangle> street_rectangles;
    std::list<std::pair<std::string, std::string>> way_point_edges_1_lane;
    std::map<WayPointLocation, std::list<std::pair<FixedArray<float, 3>, FixedArray<float, 3>>>> way_point_edges_2_lanes;
    {
        ResourceNameCycle street_lights{scene_node_resources, config.street_light_resource_names};

        // draw_nodes(vertices, nodes, ways);
        // draw_test_lines(vertices, 0.02);
        // draw_ways(vertices, nodes, ways, 0.002);
        try {
            DrawStreets{DrawStreetsInput{
                osm_triangle_lists,
                air_triangle_lists,
                resource_instance_positions_,
                object_resource_descriptors_,
                hitboxes_,
                street_rectangles,
                height_bindings,
                way_point_edges_1_lane,
                way_point_edges_2_lanes,
                tunnel_pipe_cva->triangles,
                tunnel_bdry_cva->triangles,
                nodes,
                ways,
                config.scale,
                config.uv_scale_street,
                config.default_street_width,
                config.default_lane_width,
                config.default_tunnel_pipe_width,
                config.default_tunnel_pipe_height,
                config.only_raceways,
                config.highway_name_pattern,
                config.excluded_highways,
                config.path_tags,
                config.curb_alpha,
                config.curb2_alpha,
                config.curb_uv_x,
                config.curb2_uv_x,
                config.curb_color,
                street_lights,
                config.with_height_bindings,
                config.driving_direction,
                config.layer_heights
            }};
        } catch (const TriangleException& e) {
            handle_triangle_exception(e, "Could not draw streets");
        }
    }

    if (config.with_buildings) {
        LOG_INFO("draw_building_walls (facade)");
        draw_building_walls(
            tls_buildings,
            nullptr,            // Steiner points not required due to existance of ground triangles.
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
        LOG_INFO("draw building ground");
        draw_buildings_ceiling_or_ground(
            osm_triangle_lists.tls_buildings_ground,
            Material(),
            buildings,
            nodes,
            config.scale,
            config.uv_scale_ceiling,
            config.max_wall_width,
            DrawBuildingPartType::GROUND);
    }

    auto hole_triangles = osm_triangle_lists.hole_triangles();
    StreetBvh ground_steet_bvh{hole_triangles};

    if (config.forest_outline_tree_distance != INFINITY && !config.tree_resource_names.empty()) {
        ResourceNameCycle rnc{scene_node_resources, config.tree_resource_names};
        LOG_INFO("add_trees_to_forest_outlines");
        add_trees_to_forest_outlines(
            resource_instance_positions_,
            object_resource_descriptors_,
            hitboxes_,
            steiner_points,
            rnc,
            config.min_dist_to_road,
            ground_steet_bvh,
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
            config.min_dist_to_road,
            ground_steet_bvh,
            nodes,
            config.scale);
    }
    {
        LOG_INFO("draw_building_walls (barrier)");
        draw_building_walls(
            tls_wall_barriers,
            &steiner_points,
            Material{
                .blend_mode = config.barrier_blend_mode,
                .occluder_type = OccluderType::BLACK,
                .aggregate_mode = AggregateMode::ONCE,
                .is_small = false,
                .cull_faces = false,
                .draw_distance_noperations = 1000},
            wall_barriers,
            nodes,
            config.scale,
            config.uv_scale_barrier_wall,
            config.max_wall_width,
            { config.barrier_texture });
    }

    std::vector<FixedArray<float, 2>> map_outer_contour = get_map_outer_contour(
        nodes,
        ways);
    BoundingInfo bounding_info{map_outer_contour, nodes, 0.1f};

    if (config.with_terrain) {
        // save_obj("/tmp/tl_tunnel_entrance.obj", IndexedFaceSet<float, size_t>{osm_triangle_lists.tl_tunnel_entrance->triangles_});
        // save_obj("/tmp/tl_street.obj", IndexedFaceSet<float, size_t>{osm_triangle_lists.tl_street->triangles_});
        // save_obj("/tmp/tl_tunnel_bdry.obj", IndexedFaceSet<float, size_t>{air_triangle_lists.tl_tunnel_bdry->triangles_});
        if (const char* prefix = getenv("MESH_AROUND_PREFIX"); prefix != nullptr) {
            std::vector<float> coords = string_to_vector(getenv("MESH_AROUND_POS"), safe_stof);
            if (coords.size() != 2) {
                throw std::runtime_error("MESH_AROUND_POS does not have length 2");
            }
            {
                FixedArray<double, 3> pos{coords[0], coords[1], 0.f};
                auto m = get_geographic_mapping(SceneNode());
                std::cerr.precision(15);
                std::cerr << "Saving mesh around " << pos << " | " << m.transform(pos) << std::endl;
            }
            for (float r : string_to_vector(getenv("MESH_AROUND_RADIUSES"), safe_stof)) {
                plot_mesh(                         
                    ArrayShape{2000, 2000},         // image_size
                    1,                              // line_thickness
                    4,                              // point_size
                    get_triangles_around(           // triangles
                        hole_triangles,
                        {coords[0], coords[1]},
                        r),
                    {},                             // contour
                    {{coords[0], coords[1], 0.f}},  // highlighted_nodes
                    {}                              // crossed_nodes
                    ).T().reversed(0).save_to_file(std::string(prefix) + "_r_" + std::to_string(r) + ".ppm");
            }
        }
        // {
        //     std::list<const FixedArray<ColoredVertex, 3>*> tf;
        //     for (auto& t : hole_triangles) {
        //         tf.push_back(&t);
        //     }
        //     plot_mesh_svg("/tmp/plt.svg", 800, 800, tf, {}, {});
        // }
        steiner_points = removed_duplicates(steiner_points, false);  // false = verbose
        LOG_INFO("add_street_steiner_points");
        add_street_steiner_points(
            steiner_points,
            ground_steet_bvh,
            StreetBvh{air_triangle_lists.hole_triangles()},
            bounding_info,
            config.scale,
            config.steiner_point_distances_road,
            config.steiner_point_distances_steiner);
        // {
        //     std::list<const FixedArray<ColoredVertex, 3>*> tf;
        //     for (auto& t : osm_triangle_lists.tl_entrance.at(EntranceType::BRIDGE)->triangles_) {
        //         tf.push_back(&t);
        //     }
        //     std::list<FixedArray<float, 3>> highlighted_nodes;
        //     for (auto& si : steiner_points) {
        //         highlighted_nodes.push_back(si.position);
        //     }
        //     plot_mesh_svg("/tmp/plt.svg", 800, 800, tf, {}, highlighted_nodes);
        // }
        LOG_INFO("triangulate_terrain_or_ceilings");
        try {
            triangulate_terrain_or_ceilings(
                *tl_terrain_,
                bounding_info,
                steiner_points,
                map_outer_contour,
                hole_triangles,
                terrain_region_contours,
                config.scale,
                config.uv_scale_terrain,
                0,
                terrain_color,
                getenv_default("CONTOUR_FILENAME", ""),
                config.default_terrain_type);
        } catch (const p2t::PointException& e) {
            handle_point_exception(e, "Could not triangulate terrain");
        } catch (const EdgeException& e) {
            handle_edge_exception(e, "Could not triangulate terrain");
        } catch (const TriangleException& e) {
            handle_triangle_exception(e, "Could not triangulate terrain");
        }
        if (config.blend_street) {
            auto& tl = *osm_triangle_lists.tl_terrain_visuals[config.default_terrain_type];
            for (const auto& t : osm_triangle_lists.street_triangles()) {
                tl.draw_triangle_wo_normals(
                    {t(0).position(0), t(0).position(1), 0.f},
                    {t(1).position(0), t(1).position(1), 0.f},
                    {t(2).position(0), t(2).position(1), 0.f},
                    terrain_color,
                    terrain_color,
                    terrain_color,
                    {t(0).position(0) / config.scale * config.uv_scale_terrain, t(0).position(1) / config.scale * config.uv_scale_terrain},
                    {t(1).position(0) / config.scale * config.uv_scale_terrain, t(1).position(1) / config.scale * config.uv_scale_terrain},
                    {t(2).position(0) / config.scale * config.uv_scale_terrain, t(2).position(1) / config.scale * config.uv_scale_terrain});
            }
        }
        // save_obj("/tmp/tl_terrain.obj", IndexedFaceSet<float, size_t>{tl_terrain_->triangles_});
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
        draw_buildings_ceiling_or_ground(
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
            config.max_wall_width,
            DrawBuildingPartType::CEILING);
    }
    if (config.remove_backfacing_triangles) {
        LOG_INFO("remove_backfacing_triangles");
        const char* prefix = getenv("BACKFACING_TRIANGLES_PREFIX");
        size_t i = 0;
        for (auto& l : std::list{&osm_triangle_lists, &air_triangle_lists}) {
            delete_backfacing_triangles(
                l->tls_no_backfaces(),
                prefix == nullptr
                    ? ""
                    : prefix + std::to_string(i) + ".ppm");
            ++i;
        }
    }

    smoothen_and_apply_heightmap(
        config,
        height_bindings,
        nodes,
        ways,
        normalized_points,
        tls_buildings,
        tls_wall_barriers,
        osm_triangle_lists,
        air_triangle_lists,
        object_resource_descriptors_,
        resource_instance_positions_,
        hitboxes_,
        steiner_points,
        street_rectangles,
        way_point_edges_2_lanes);

    // save_obj("/tmp/tl_terrain1.obj", IndexedFaceSet<float, size_t>{tl_terrain_->triangles_});
    // {
    //     auto plot = [](const std::string& prefix, const OsmTriangleLists& lsts){
    //         for (const auto& l : lsts.tl_street.list()) {
    //             save_obj(
    //                 prefix + std::to_string((unsigned int)l.road_properties.type) + "-" + std::to_string(l.road_properties.nlanes) + ".obj",
    //                 IndexedFaceSet<float, size_t>{l.styled_road.triangle_list->triangles_});
    //         }
    //     };
    //     plot("/tmp/tl_street_ground", osm_triangle_lists);
    //     plot("/tmp/tl_street_air", air_triangle_lists);
    // }

    if (!air_triangle_lists.tl_tunnel_bdry->triangles_.empty()) {
        // mesh_subtract(osm_triangle_lists.tl_terrain->triangles_, air_triangle_lists.tl_tunnel_bdry->triangles_);
        // osm_triangle_lists.tl_terrain->calculate_triangle_normals();
        // save_obj("/tmp/terrain.obj", IndexedFaceSet<float, size_t>{osm_triangle_lists.tl_terrain->triangles_});
        // save_obj("/tmp/tunnels.obj", IndexedFaceSet<float, size_t>{air_triangle_lists.tl_tunnel_bdry->triangles_});
    }
    // If extrude_air_curb_amount is not NAN,
    // boundaries have to be calculated at the ends of
    // ends of air and ground street.
    try {
        if (std::isnan(config.extrude_air_curb_amount)) {
            osm_triangle_lists.insert(air_triangle_lists);
        } else if (config.extrude_air_curb_amount != 0) {
            TriangleList::extrude(
                *air_triangle_lists.tl_street_curb[RoadType::STREET],
                {air_triangle_lists.tl_street_curb[RoadType::STREET]},
                nullptr,
                nullptr,
                config.extrude_air_curb_amount * config.scale,
                config.scale,
                config.uv_scale_street,
                config.uv_scale_street);
            if (air_triangle_lists.tl_street_curb.contains(RoadType::PATH)) {
                TriangleList::extrude(
                    *air_triangle_lists.tl_street_curb[RoadType::PATH],
                    {air_triangle_lists.tl_street_curb[RoadType::PATH]},
                    nullptr,
                    nullptr,
                    config.extrude_air_curb_amount * config.scale,
                    config.scale,
                    config.uv_scale_street,
                    config.uv_scale_street);
            }
        }
        if (config.extrude_curb_amount != 0) {
            TriangleList::extrude(
                *osm_triangle_lists.tl_street_curb[RoadType::STREET],
                {osm_triangle_lists.tl_street_curb[RoadType::STREET]},
                nullptr,
                nullptr,
                config.extrude_curb_amount * config.scale,
                config.scale,
                config.uv_scale_street,
                config.uv_scale_street);
            if (air_triangle_lists.tl_street_curb.contains(RoadType::PATH)) {
                TriangleList::extrude(
                    *osm_triangle_lists.tl_street_curb[RoadType::PATH],
                    {osm_triangle_lists.tl_street_curb[RoadType::PATH]},
                    nullptr,
                    nullptr,
                    config.extrude_curb_amount * config.scale,
                    config.scale,
                    config.uv_scale_street,
                    config.uv_scale_street);
            }
        }
        if (config.extrude_wall_amount != 0) {
            TriangleList::extrude(
                *osm_triangle_lists.tl_street[RoadProperties{.type = RoadType::WALL, .nlanes = 1}].triangle_list,
                {osm_triangle_lists.tls_wall_wo_curb()},
                nullptr,
                nullptr,
                config.extrude_wall_amount * config.scale,
                config.scale,
                1.f,
                config.uv_scale_highway_wall);
        }
        if (std::isnan(config.extrude_air_curb_amount)) {
            raise_streets(
                osm_triangle_lists.tls_street_wo_curb(),
                osm_triangle_lists.tls_wo_subtraction_and_water(),
                config.scale,
                config.raise_streets_amount);
        } else {
            raise_streets(
                TriangleList::concatenated(
                    osm_triangle_lists.tls_street_wo_curb(),
                    air_triangle_lists.tls_street_wo_curb()),
                TriangleList::concatenated(
                    osm_triangle_lists.tls_wo_subtraction_and_water(),
                    air_triangle_lists.tls_wo_subtraction_and_water()),
                config.scale,
                config.raise_streets_amount);
        }
        if (config.extrude_grass_amount != 0) {
            TriangleList::extrude(
                *osm_triangle_lists.tl_terrain_extrusion[TerrainType::GRASS],
                {(*osm_triangle_lists.tl_terrain)[TerrainType::GRASS]},
                nullptr,
                nullptr,
                config.extrude_grass_amount * config.scale,
                config.scale,
                config.uv_scale_street,
                config.uv_scale_street);
        }
    } catch (const EdgeException& e) {
        handle_edge_exception(e, "Extrusion failed");
    } catch (const TriangleException& e) {
        handle_triangle_exception(e, "Extrusion failed");
    }
    std::set<OrderableFixedArray<float, 3>> boundary_vertices;
    // Compute boundary vertices.
    if ((config.extrude_street_amount != 0) || (config.extrude_air_support_amount != 0))
    {
        std::set<OrderableFixedArray<float, 3>> terrain_vertices;
        for (const auto& l : osm_triangle_lists.tl_terrain->map()) {
            for (const auto& t : l.second->triangles_) {
                for (const auto& v : t.flat_iterable()) {
                    terrain_vertices.insert(OrderableFixedArray{v.position});
                }
            }
        }
        for (const auto& l : osm_triangle_lists.tls_street()) {
            for (const auto& t : l->triangles_) {
                for (const auto& v : t.flat_iterable()) {
                    if (terrain_vertices.contains(OrderableFixedArray{v.position})) {
                        boundary_vertices.insert(OrderableFixedArray{v.position});
                    }
                }
            }
        }
    }
    if (config.extrude_street_amount != 0) {
        check_curb_validity(config.curb_alpha, config.curb2_alpha);
        if (config.curb_alpha == 1) {
            TriangleList::extrude(
                *osm_triangle_lists.tl_terrain_extrusion[config.default_terrain_type],
                osm_triangle_lists.tls_street_wo_curb(),
                nullptr,
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
            auto do_extrude = [&config, &boundary_vertices]
                (OsmTriangleLists& triangle_lists)
            {
                std::list<std::shared_ptr<TriangleList>> source_triangles{triangle_lists.tls_curb_only()};
                TriangleList::extrude(
                    *triangle_lists.tl_street_curb[RoadType::STREET],
                    triangle_lists.tls_street_wo_curb(),
                    &source_triangles,
                    &boundary_vertices,
                    config.extrude_street_amount * config.scale,
                    config.scale,
                    1,
                    config.uv_scale_terrain);
            };
            if (std::isnan(config.extrude_air_curb_amount)) {
                do_extrude(osm_triangle_lists);
            } else {
                do_extrude(osm_triangle_lists);
                do_extrude(air_triangle_lists);
            }
        }
    }

    {
        // Extrude air support and raise tunnel crossings.
        const auto& air_or_osm = std::isnan(config.extrude_air_curb_amount)
            ? osm_triangle_lists
            : air_triangle_lists;

        LOG_INFO("flip air-support normals");
        // Must be after "delete_backfacing_triangles".
        air_or_osm.tl_air_support->flip();
        air_or_osm.tl_tunnel_crossing->flip();
        // Raise tunnel crossings.
        for (auto& t : air_or_osm.tl_tunnel_crossing->triangles_) {
            for (auto& v : t.flat_iterable()) {
                v.position(2) += config.default_tunnel_pipe_height * config.scale;
            }
        }

        // save_obj("/tmp/tl_terrain0.obj", IndexedFaceSet<float, size_t>{tl_terrain_->triangles_});
        std::set<const FixedArray<ColoredVertex, 3>*> triangles_to_delete;
        for (const auto& t : air_or_osm.tl_air_support->triangles_) {
            if (boundary_vertices.contains(OrderableFixedArray{t(0).position}) ||
                boundary_vertices.contains(OrderableFixedArray{t(1).position}) ||
                boundary_vertices.contains(OrderableFixedArray{t(2).position}))
            {
                triangles_to_delete.insert(&t);
            }
        }
        if (config.extrude_air_support_amount != 0) {
            TriangleList::extrude(
                *air_or_osm.tl_air_support,
                {air_or_osm.tl_air_support},
                nullptr,
                &boundary_vertices,
                config.extrude_air_support_amount * config.scale,
                config.scale,
                1,
                config.uv_scale_terrain);
            air_or_osm.tl_air_support->triangles_.remove_if([&triangles_to_delete](const FixedArray<ColoredVertex, 3>& t){
                return triangles_to_delete.contains(&t);
            });
        }
    }

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
            *(*tl_terrain_)[TerrainType::GRASS],
            config.scale,
            config.much_grass_distance);
    }
    calculate_spawn_points(
        spawn_points_,
        street_rectangles,
        config.scale,
        config.driving_direction);
    if (false) {
        resource_instance_positions_.clear();
        for (const auto& p : spawn_points_) {
            resource_instance_positions_["grass12y"].push_back(ResourceInstanceDescriptor{.position = p.position});
        }
        for (const auto& p : osm_triangle_lists.tls_street()) {
            for (const auto& t : p->triangles_) {
                for (const auto& v : t.flat_iterable()) {
                    auto it = height_bindings.find(OrderableFixedArray<float, 2>{v.position(0), v.position(1)});
                    if (it != height_bindings.end()) {
                        if (it->second == "1792772911") {  // node 1792772911 4002287619
                            resource_instance_positions_["grass12y"].push_back(ResourceInstanceDescriptor{.position = v.position});
                        }
                    }
                }
            }
        }
    }
    {
        std::list<Building> way_point_lines = get_buildings_or_wall_barriers(
            BuildingType::WAYPOINTS,
            ways,
            0,  // building_bottom
            0); // default_building_top
        calculate_waypoint_adjacency(
            way_points_[WayPointLocation::STREET],
            way_point_lines,
            way_point_edges_1_lane,
            way_point_edges_2_lanes[WayPointLocation::STREET],
            nodes,
            config.scale);
        calculate_waypoint_adjacency(
            way_points_[WayPointLocation::SIDEWALK],
            way_point_lines,
            {},
            way_point_edges_2_lanes[WayPointLocation::SIDEWALK],
            nodes,
            config.scale);
    }
    if (const char* wf = getenv("WAYPOINT_DEBUG_PREFIX"); (wf != nullptr)) {
        way_points_.at(WayPointLocation::STREET).plot(std::string(wf) + "street.svg", 600, 600, 0.1);
        way_points_.at(WayPointLocation::SIDEWALK).plot(std::string(wf) + "sidewalk.svg", 600, 600, 0.1);
    }

    if (!std::isnan(config.extrude_air_curb_amount)) {
        for (auto& l : air_triangle_lists.tl_street_curb.map()) {
            air_triangle_lists.tl_air_street_curb[l.first]->triangles_ = std::move(l.second->triangles_);
        }
        osm_triangle_lists.insert(air_triangle_lists);
    }

    // Normals are invalid after "apply_heightmap"
    for (auto& l2 : osm_triangle_lists.tls_wo_subtraction_and_water()) {
        l2->calculate_triangle_normals();
    }
    TriangleList::convert_triangle_to_vertex_normals(osm_triangle_lists.tls_with_vertex_normals());
    TriangleList::convert_triangle_to_vertex_normals(tls_wall_barriers);

    // save_obj("/tmp/tl_terrain_final.obj", IndexedFaceSet<float, size_t>{osm_triangle_lists.tl_terrain->triangles_});
    // save_obj("/tmp/tl_tunnel_pipe_final.obj", IndexedFaceSet<float, size_t>{osm_triangle_lists.tl_tunnel_pipe->triangles_});
    // save_obj("/tmp/tl_street_final.obj", IndexedFaceSet<float, size_t>{osm_triangle_lists.tl_street->triangles_});

    tls_no_grass_ = osm_triangle_lists.tls_no_grass();

    std::list<std::shared_ptr<TriangleList>> tls_all;
    if (!config.water_texture.empty()) {
        std::list<std::pair<WaterType, std::list<FixedArray<float, 3>>>> water_contours =
            get_water_region_contours(nodes, ways);
        LOG_INFO("triangulate_water");
        try {
            triangulate_water(
                osm_triangle_lists.tl_water,
                bounding_info,
                {},                     // steiner_points
                map_outer_contour,
                {},                     // hole_triangles
                water_contours,
                config.scale,
                1 / 100.f,              // uv_scale
                config.water_height,
                terrain_color,
                getenv_default("CONTOUR_FILENAME", ""),
                WaterType::UNDEFINED);
        } catch (const p2t::PointException& e) {
            handle_point_exception(e, "Could not triangulate water");
        } catch (const EdgeException& e) {
            handle_edge_exception(e, "Could not triangulate water");
        } catch (const TriangleException& e) {
            handle_triangle_exception(e, "Could not triangulate water");
        }
        tls_all = std::move(osm_triangle_lists.tls_wo_subtraction_w_water());
    } else {
        tls_all = std::move(osm_triangle_lists.tls_wo_subtraction_and_water());
    }
    for (auto& l : std::list<const std::list<std::shared_ptr<TriangleList>>*>{
            &tls_all,
            &tls_buildings,
            &tls_wall_barriers})
    {
        for (auto& l2 : *l) {
            if (!l2->triangles_.empty()) {
                cvas_.push_back(l2->triangle_array());
            }
        }
    }
}

OsmMapResource::~OsmMapResource()
{}

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
    if (!near_grass_resource_names_.empty() && (much_near_grass_distance_ != INFINITY)) {
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

TransformationMatrix<double, 3> OsmMapResource::get_geographic_mapping(const SceneNode& scene_node) const
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
        0.f};
    return TransformationMatrix<double, 3>{inv((scene_node.absolute_model_matrix().casted<double>() * m3).affine())};
}

std::list<SpawnPoint> OsmMapResource::spawn_points() const {
    return spawn_points_;
}

std::map<WayPointLocation, PointsAndAdjacency<float, 2>> OsmMapResource::way_points() const {
    return way_points_;
}
