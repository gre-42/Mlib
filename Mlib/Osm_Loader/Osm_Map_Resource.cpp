#include "Osm_Map_Resource.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Geography/Geographic_Coordinates.hpp>
#include <Mlib/Geometry/Coordinates/Normalized_Points_Fixed.hpp>
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>
#include <Mlib/Geometry/Exceptions/Point_Exception.hpp>
#include <Mlib/Geometry/Exceptions/Triangle_Exception.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Mesh/Bone.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Modulo_Uv.cpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Plot.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Geometry/Mesh/Save_Obj.hpp>
#include <Mlib/Geometry/Mesh/Terrain_Uv.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Largest_Cosine.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Mesh/Triangles_Around.hpp>
#include <Mlib/Geometry/Mesh/Up_Axis.hpp>
#include <Mlib/Images/StbImage1.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Navigation/NavigationMeshBuilder.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Add_Grass_Inside_Triangles.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Add_Grass_on_Steiner_Points.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Add_Models_To_Model_Nodes.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Add_Street_Steiner_Points.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Add_Trees_To_Forest_Outlines.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Add_Trees_To_Tree_Nodes.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Apply_Displacement_Map.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Bounding_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Calculate_Spawn_Points.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Calculate_Waypoint_Adjacency.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Compute_Building_Area.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Delete_Backfacing_Triangles.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Boundary_Barriers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Building_Part_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Building_Walls.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Buildings_Ceiling_Or_Ground.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Ceilings.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Into_Street_Rectangles.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Roofs.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Streets.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Wall_Barriers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Facade_Texture_Cycle.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Generate_Racing_Line_Playback.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Get_Buildings_Or_Wall_Barriers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Get_Map_Outer_Contour.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Get_Terrain_Region_Contours.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Get_Terrain_Way_Points.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Get_Water_Region_Contours.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Ground_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Load_Racing_Line_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Material_Colors.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Node_Height_Binding.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Nodes_And_Ways.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Resource_Config.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Triangle_Lists.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Parse_Osm_Xml.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Project_Nodes_Onto_Ways.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Racing_Line_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Report_Osm_Problems.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Road_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Smoothen_And_Apply_Heightmap.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Smoothen_Ways.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Rectangle.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Way_Point.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Styled_Road.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Terrain_Way_Points.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Triangulate_Terrain_Or_Ceilings.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Vertex_Height_Binding.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Water_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Way_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Wayside_Resource_Names.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Collidable_Triangle_Sampler.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Renderable_Triangle_Sampler.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Resource_Name_Cycle.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Sample_Triangle_Interior_Instances.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Triangles.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Descriptors/Object_Resource_Descriptor.hpp>
#include <Mlib/Scene_Graph/Descriptors/Resource_Instance_Descriptor.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <cereal/access.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>
#include <mutex>
#include <poly2tri/point_exception.hpp>

// #undef LOG_FUNCTION
// #undef LOG_INFO
// #define LOG_FUNCTION(msg) ::Mlib::Log log(msg, "LOG_OSM_MAP")
// #define LOG_INFO(msg) log.info(msg)

using namespace Mlib;

OsmMapResource::OsmMapResource(
    SceneNodeResources& scene_node_resources,
    const OsmResourceConfig& config,
    const std::string& debug_prefix)
: hri_{ scene_node_resources, { 90.f * degrees, 0.f, 0.f }, config.scale },
  scene_node_resources_{ scene_node_resources },
  scale_{ config.scale },
  terrain_styles_{ config.triangle_sampler_resource_config }
{
    LOG_FUNCTION("OsmMapResource::OsmMapResource");
    NodesAndWays naws_or;
    NormalizedPointsFixed<double> normalized_points{ScaleMode::NONE, OffsetMode::CENTERED};

    parse_osm_xml(
        config.filename,
        config.scale,
        normalized_points,
        normalization_matrix_,
        naws_or.nodes,
        naws_or.ways);
    
    NodesAndWays naws_smooth = smoothen_ways(
        naws_or,
        config.smoothed_highways,
        config.default_street_width,
        config.default_lane_width,
        config.scale,
        config.max_smooth_highway_length);
    const std::map<std::string, Node>& nodes = naws_smooth.nodes;
    const std::map<std::string, Way>& ways = naws_smooth.ways;
    std::map<std::string, Node>& mnodes = naws_smooth.nodes;

    OsmTriangleLists osm_triangle_lists{config, ""};
    OsmTriangleLists air_triangle_lists{config, "_air"};
    tl_terrain_ = osm_triangle_lists.tl_terrain;
    std::list<std::shared_ptr<TriangleList<double>>> tls_buildings;
    std::list<std::shared_ptr<TriangleList<double>>> tls_wall_barriers;
    std::map<OrderableFixedArray<double, 2>, NodeHeightBinding> node_height_bindings;
    std::map<const FixedArray<double, 3>*, VertexHeightBinding<double>> vertex_height_bindings;
    std::list<SteinerPointInfo> steiner_points;
    std::list<StreetRectangle> street_rectangles;
    std::map<WayPointLocation, std::list<std::pair<StreetWayPoint, StreetWayPoint>>> way_point_edge_descriptors;
    {
        auto model_triangles = [&scene_node_resources](const std::string& resource_name) -> std::vector<FixedArray<ColoredVertex<float>, 3>>& {
            auto& scvas = scene_node_resources.get_physics_arrays(resource_name)->scvas;
            auto& dcvas = scene_node_resources.get_physics_arrays(resource_name)->dcvas;
            if (scvas.size() != 1) {
                THROW_OR_ABORT('"' + resource_name + "\" does not have exactly one single-precision mesh");
            }
            if (!dcvas.empty()) {
                THROW_OR_ABORT('"' + resource_name + "\" has a double-precision mesh");
            }
            return scvas.front()->triangles;
        };
        auto& tunnel_pipe = model_triangles(config.tunnel_pipe_resource_name);
        auto& tunnel_bdry = model_triangles(config.tunnel_bdry_resource_name);
        std::list<FixedArray<FixedArray<double, 2>, 2>> way_segments;
        ResourceNameCycle street_lights{ config.street_light_resource_names };
        RacingLineBvh racing_line_bvh;
        if (!config.racing_line_track.empty()) {
            load_racing_line_bvh(config.racing_line_track, normalization_matrix_, racing_line_bvh);
        }

        // draw_nodes(vertices, nodes, ways);
        // draw_test_lines(vertices, 0.02);
        // draw_ways(vertices, nodes, ways, 0.002);
        try {
            DrawStreets{DrawStreetsInput{
                scene_node_resources,
                osm_triangle_lists,
                air_triangle_lists,
                *hri_.bri,
                street_rectangles,
                node_height_bindings,
                way_point_edge_descriptors,
                tunnel_pipe,
                tunnel_bdry,
                way_segments,
                racing_line_bvh,
                config.street_surface_central_resource_names,
                config.street_surface_endpoint0_resource_names,
                config.street_surface_endpoint1_resource_names,
                config.street_bumps_central_resource_names,
                config.street_bumps_endpoint0_resource_names,
                config.street_bumps_endpoint1_resource_names,
                nodes,
                ways,
                config.scale,
                config.uv_scales_street,
                config.uv_scale_crossings,
                config.default_street_width,
                config.default_lane_width,
                config.default_tunnel_pipe_width,
                config.default_tunnel_pipe_height,
                config.only_raceways_and_walls,
                config.highway_name_pattern,
                config.excluded_highways,
                config.path_tags,
                config.curb_alpha,
                config.curb2_alpha,
                config.curb_uv,
                config.curb2_uv,
                config.curb_color,
                config.racing_line_width_x,
                config.racing_line_scale_y,
                street_lights,
                config.with_height_bindings,
                config.driving_direction,
                config.layer_heights
            }};
        } catch (const PointException<double, 3>& e) {
            handle_point_exception3(e, "Could not draw streets");
        } catch (const TriangleException<double>& e) {
            handle_triangle_exception(e, "Could not draw streets");
        }
        try {
            project_nodes_onto_ways(mnodes, way_segments, config.scale);
        } catch (const PointException<double, 2>& e) {
            handle_point_exception2(e, "Could not project nodes onto way");
        }
    }

    report_osm_problems(nodes, ways);

    std::list<Building> buildings;
    std::list<Building> wall_barriers;
    if (config.with_buildings || config.with_roofs || config.with_ceilings) {
        FacadeTextureCycle ftc{ config.facade_textures };
        buildings = get_buildings_or_wall_barriers(
            BuildingType::BUILDING,
            ways,
            config.building_bottom,
            config.default_building_top,
            config.uv_scale_facade,
            config.socle_textures,
            ftc);
        compute_building_area(
            buildings,
            nodes,
            config.scale);
    }
    {
        FacadeTextureCycle ftc({});
        wall_barriers = get_buildings_or_wall_barriers(
            BuildingType::WALL_BARRIER,
            ways,
            config.building_bottom,
            config.default_barrier_top,
            config.uv_scale_barrier_wall,
            {},
            ftc);
    }

    std::list<std::pair<TerrainType, std::list<FixedArray<double, 2>>>> terrain_region_contours =
        get_terrain_region_contours(nodes, ways);
    WayBvh terrain_region_contours_bvh;
    for (const auto& [_, contour] : terrain_region_contours) {
        terrain_region_contours_bvh.add_path(contour);
    }

    if (config.with_buildings) {
        for (const auto& bu : buildings) {
            if (bu.way.nd.empty()) {
                std::cerr << "Building " << bu.id << " is empty" << std::endl;
            } else if (bu.way.nd.front() != bu.way.nd.back()) {
                std::cerr << "Building " << bu.id << " has no closed outline" << std::endl;
            }
        }
        LOG_INFO("draw_building_walls (facade)");
        draw_building_walls(
            tls_buildings,
            nullptr,            // Steiner points not required due to existance of ground triangles.
            vertex_height_bindings,
            Material{
                .occluder_pass = ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC,
                .aggregate_mode = AggregateMode::ONCE,
                .emissivity = OrderableFixedArray{WALL_EMISSIVITY * config.emissivity_factor},
                .ambience = OrderableFixedArray{WALL_AMBIENCE * config.ambience_factor},
                .diffusivity = OrderableFixedArray{WALL_DIFFUSIVITY * config.diffusivity_factor},
                .specularity = OrderableFixedArray{WALL_SPECULARITY * config.specularity_factor},
                .draw_distance_noperations = 1000},
            buildings,
            nodes,
            config.scale,
            config.uv_scale_facade,
            config.max_wall_width,
            config.extrusion_ambient_occlusion,
            config.height_colors);
        LOG_INFO("draw building ground");
        draw_buildings_ceiling_or_ground(
            osm_triangle_lists.tls_buildings_ground,
            Material(),
            buildings,
            nodes,
            config.scale,
            config.uv_scale_ceiling,
            1.f,                     // uv_period
            config.max_wall_width,
            DrawBuildingPartType::GROUND);
    }

    auto all_hole_triangles = osm_triangle_lists.all_hole_triangles();
    auto street_hole_triangles = osm_triangle_lists.street_hole_triangles();
    auto building_hole_triangles = osm_triangle_lists.building_hole_triangles();
    auto ocean_ground_triangles = osm_triangle_lists.ocean_ground_triangles();
    StreetBvh all_holes_bvh{all_hole_triangles};
    StreetBvh ground_street_bvh{street_hole_triangles};

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
            scene_node_resources,
            *hri_.bri,
            nodes,
            ways,
            config.raceway_beacon_distance,
            config.scale);
    }
    {
        LOG_INFO("draw_wall_barriers");
        draw_wall_barriers(
            tls_wall_barriers,
            &steiner_points,
            Material{
                .occluder_pass = ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC,
                .aggregate_mode = AggregateMode::ONCE,
                .cull_faces = false,
                .reorient_uv0 = true,
                .emissivity = OrderableFixedArray{WALL_EMISSIVITY * config.emissivity_factor},
                .ambience = OrderableFixedArray{WALL_AMBIENCE * config.ambience_factor},
                .diffusivity = OrderableFixedArray{WALL_DIFFUSIVITY * config.diffusivity_factor},
                .specularity = OrderableFixedArray{WALL_SPECULARITY * config.specularity_factor},
                .draw_distance_noperations = 1000},
            wall_barriers,
            nodes,
            config.scale,
            config.uv_scale_barrier_wall,
            config.max_wall_width,
            config.barrier_styles);
    }
    if (!config.boundary_barrier_style.empty()) {
        LOG_INFO("draw_boundary_barriers");
        try {
            draw_boundary_barriers(
                tls_wall_barriers,
                street_hole_triangles,
                Material{
                    .occluder_pass = ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC,
                    .aggregate_mode = AggregateMode::ONCE,
                    .cull_faces = false,
                    .reorient_uv0 = true,
                    .emissivity = OrderableFixedArray{WALL_EMISSIVITY * config.emissivity_factor},
                    .ambience = OrderableFixedArray{WALL_AMBIENCE * config.ambience_factor},
                    .diffusivity = OrderableFixedArray{WALL_DIFFUSIVITY * config.diffusivity_factor},
                    .specularity = OrderableFixedArray{WALL_SPECULARITY * config.specularity_factor},
                    .draw_distance_noperations = 1000},
                config.scale,
                config.uv_scale_barrier_wall,
                config.boundary_barrier_height,
                config.barrier_styles.get(config.boundary_barrier_style));
        } catch (const EdgeException<double>& e) {
            handle_edge_exception(e, "Could not draw boundary barriers");
        }
    }

    std::vector<FixedArray<double, 2>> map_outer_contour = get_map_outer_contour(
        nodes,
        ways);
    BoundingInfo bounding_info{map_outer_contour, nodes, 0.1f};

    auto draw_terrain_triangles = [&config](TriangleList<double>& dest, const std::list<FixedArray<ColoredVertex<double>, 3>>& source){
        for (const auto& t : source) {
            auto uv = terrain_uv<double>(
                t(0).position,
                t(1).position,
                t(2).position,
                config.scale,
                config.uv_scale_terrain,
                config.uv_period_terrain,
                UpAxis::Z);
            dest.draw_triangle_wo_normals(
                t(0).position,
                t(1).position,
                t(2).position,
                terrain_color,
                terrain_color,
                terrain_color,
                uv(0),
                uv(1),
                uv(2));
        }
    };
    if (config.with_terrain) {
        // save_obj("/tmp/tl_tunnel_entrance.obj", IndexedFaceSet<float, size_t>{osm_triangle_lists.tl_tunnel_entrance->triangles_});
        // {
        //     size_t i = 0;
        //     for (const auto& e : osm_triangle_lists.tl_street.list()) {
        //         save_obj("/tmp/tl_street_" + std::to_string(i) + ".obj", IndexedFaceSet<float, size_t>{e.styled_road.triangle_list->triangles_});
        //         ++i;
        //         for (const auto& t : e.styled_road.triangle_list->triangles_) {
        //             for (const auto& v : t.flat_iterable()) {
        //                 if (max(abs(v.position - FixedArray<float, 3>{-0.627685, 1.276513, 0.000000})) < 1e-6) {
        //                     std::cerr << std::setprecision(15) << v << std::endl;
        //                 }
        //             }
        //         }
        //     }
        // }
        // {
        //     size_t i = 0;
        //     for (const auto& e : osm_triangle_lists.tls_crossing_only()) {
        //         save_obj("/tmp/tl_crossing_" + std::to_string(i) + ".obj", IndexedFaceSet<float, size_t>{e->triangles_});
        //         ++i;
        //     }
        // }
        // save_obj("/tmp/tl_tunnel_bdry.obj", IndexedFaceSet<float, size_t>{air_triangle_lists.tl_tunnel_bdry->triangles_});
        if (auto prefix = try_getenv("MESH_AROUND_PREFIX"); prefix.has_value()) {
            std::vector<float> coords = string_to_vector(str_getenv("MESH_AROUND_POS"), safe_stof);
            if (coords.size() != 2) {
                THROW_OR_ABORT("MESH_AROUND_POS does not have length 2");
            }
            {
                FixedArray<double, 3> pos{coords[0], coords[1], 0.f};
                auto m = get_geographic_mapping(TransformationMatrix<double, double, 3>::identity());
                std::cerr.precision(15);
                std::cerr << "Saving mesh around " << pos << " | " << m.transform(pos) << std::endl;
            }
            for (float r : string_to_vector(str_getenv("MESH_AROUND_RADIUSES"), safe_stof)) {
                plot_mesh(                         
                    ArrayShape{2000, 2000},         // image_size
                    1,                              // line_thickness
                    4,                              // point_size
                    get_triangles_around(           // triangles
                        all_hole_triangles,
                        {coords[0], coords[1]},
                        r),
                    {},                             // contour
                    {{coords[0], coords[1], 0.f}},  // highlighted_nodes
                    {}                              // crossed_nodes
                    ).T().reversed(0).save_to_file(prefix.value() + "_r_" + std::to_string(r) + ".png");
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
            ground_street_bvh,
            terrain_region_contours_bvh,
            bounding_info,
            config.scale,
            config.steiner_point_distances_road,
            config.steiner_point_distances_steiner,
            config.min_dist_to_terrain_region);
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
                {{TerrainType::STREET_HOLE, street_hole_triangles},
                 {TerrainType::BUILDING_HOLE, building_hole_triangles},
                 {TerrainType::OCEAN_GROUND, ocean_ground_triangles}},
                terrain_region_contours,
                config.scale,
                config.uv_scale_terrain,
                config.uv_period_terrain,
                0,
                terrain_color,
                getenv_default("TERRAIN_CONTOUR_TRIANGLES_FILENAME", ""),
                getenv_default("TERRAIN_CONTOUR_FILENAME", ""),
                getenv_default("TERRAIN_TRIANGLE_FILENAME", ""),
                config.bounding_terrain_type,
                config.default_terrain_type,
                // Delete street holes because their untriangulated version is
                // used for terrain smoothing.
                {TerrainType::STREET_HOLE});
        } catch (const p2t::PointException& e) {
            handle_point_exception(e, "Could not triangulate terrain (TERRAIN_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const EdgeException<double>& e) {
            handle_edge_exception(e, "Could not triangulate terrain (TERRAIN_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const TriangleException<double>& e) {
            handle_triangle_exception(e, "Could not triangulate terrain (TERRAIN_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        }
        for (const WaysideResourceNames& ws : config.waysides) {
            LOG_INFO("add_grass_on_steiner_points");
            ResourceNameCycle rnc{ ws.resource_names };
            add_grass_on_steiner_points(
                *hri_.bri,
                rnc,
                StreetBvh{ osm_triangle_lists.no_trees_triangles() },
                StreetBvh{ air_triangle_lists.street_hole_triangles() },
                steiner_points,
                config.scale,
                ws.min_dist,
                ws.max_dist);
        }
        // Note that ditch is in the group OsmTriangleLists::tls_terrain_nosmooth().
        draw_terrain_triangles(
            *(*osm_triangle_lists.tl_terrain)[config.default_terrain_type],
            osm_triangle_lists.ditch_triangles());
        // save_obj("/tmp/tl_terrain.obj", IndexedFaceSet<float, size_t>{tl_terrain_->triangles_});
    }
    if (config.with_roofs) {
        LOG_INFO("draw_roofs");
        auto primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
        draw_roofs(
            tls_buildings,
            Material{
                .textures = { primary_rendering_resources->get_blend_map_texture(config.roof_texture) },
                .occluder_pass = ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC,
                .aggregate_mode = AggregateMode::ONCE,
                .emissivity = OrderableFixedArray{ROOF_EMISSIVITY * config.emissivity_factor},
                .ambience = OrderableFixedArray{ROOF_AMBIENCE * config.ambience_factor},
                .diffusivity = OrderableFixedArray{ROOF_DIFFUSIVITY * config.diffusivity_factor},
                .specularity = OrderableFixedArray{ROOF_SPECULARITY * config.specularity_factor},
                .draw_distance_noperations = 1000}.compute_color_mode(),
            roof_color,
            buildings,
            nodes,
            config.scale,
            config.uv_scale_roof,
            config.max_wall_width);
    }
    if (config.with_ceilings && !tls_buildings.empty()) {
        LOG_INFO("draw_ceilings");
        draw_ceilings(tls_buildings, config, buildings, nodes);
    }
    if (config.remove_backfacing_triangles) {
        LOG_INFO("remove_backfacing_triangles");
        auto prefix = try_getenv("BACKFACING_TRIANGLES_PREFIX");
        size_t i = 0;
        for (auto& l : std::list{&osm_triangle_lists, &air_triangle_lists}) {
            delete_backfacing_triangles(
                l->tls_no_backfaces(),
                !prefix.has_value()
                    ? ""
                    : prefix.value() + std::to_string(i) + ".png");
            ++i;
        }
    }

    std::list<FixedArray<double, 3>> map_outer_contour3;
    for (const auto& p : map_outer_contour) {
        map_outer_contour3.push_back(FixedArray<double, 3>{ p(0), p(1), 0.f });
    }

    try {
        LOG_INFO("smoothen and apply heightmap");
        smoothen_and_apply_heightmap(
            config,
            node_height_bindings,
            vertex_height_bindings,
            nodes,
            ways,
            normalized_points,
            tls_buildings,
            tls_wall_barriers,
            osm_triangle_lists,
            air_triangle_lists,
            VertexOutOfHeightMapBehavior::THROW,
            *hri_.bri,
            steiner_points,
            map_outer_contour3,
            street_rectangles,
            way_point_edge_descriptors);
    } catch (const PointException<double, 2>& e) {
        handle_point_exception2(e, "Could not smoothen and apply heighmap. Forgot to set map outer contour?");
    }

    if (!config.displacementmap.empty()) {
        apply_displacement_map(
            ground_street_bvh,
            osm_triangle_lists.tls_smoothed(),
            StbImage1::load_from_file(config.displacementmap).to_float_grayscale().casted<double>(),
            config.displacementmap_min,
            config.displacementmap_uv_scale,
            config.displacementmap_distance_2_z_scale,
            config.scale);
    }

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

    if (!air_triangle_lists.tl_tunnel_bdry->triangles.empty()) {
        // mesh_subtract(osm_triangle_lists.tl_terrain->triangles_, air_triangle_lists.tl_tunnel_bdry->triangles_);
        // osm_triangle_lists.tl_terrain->calculate_triangle_normals();
        // save_obj("/tmp/terrain.obj", IndexedFaceSet<float, size_t>{osm_triangle_lists.tl_terrain->triangles_});
        // save_obj("/tmp/tunnels.obj", IndexedFaceSet<float, size_t>{air_triangle_lists.tl_tunnel_bdry->triangles_});
    }
    // If extrude_air_curb_amount is not NAN,
    // boundaries have to be calculated at the ends of
    // ends of air and ground street.
    try {
        LOG_INFO("extrude curbs, walls, grass, water");
        if (std::isnan(config.extrude_air_curb_amount)) {
            // If "extrude_air_curb_amount" IS NAN,
            // insert the air triangle lists here.
            osm_triangle_lists.insert(air_triangle_lists);
        } else if (config.extrude_air_curb_amount != 0) {
            TriangleList<double>::extrude(
                *air_triangle_lists.tl_street_curb[RoadType::STREET],
                {air_triangle_lists.tl_street_curb[RoadType::STREET]},
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                config.extrude_air_curb_amount * config.scale,
                config.scale,
                config.uv_scales_street.at(RoadType::STREET),
                config.uv_scales_street.at(RoadType::STREET),
                false,  // uvs_equal_lengths
                0.f);   // ambient_occlusion
            if (air_triangle_lists.tl_street_curb.contains(RoadType::PATH)) {
                TriangleList<double>::extrude(
                    *air_triangle_lists.tl_street_curb[RoadType::PATH],
                    {air_triangle_lists.tl_street_curb[RoadType::PATH]},
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr,
                    config.extrude_air_curb_amount * config.scale,
                    config.scale,
                    config.uv_scales_street.at(RoadType::PATH),
                    config.uv_scales_street.at(RoadType::PATH),
                    false,  // uvs_equal_lengths
                    0.f);   // ambient_occlusion
            }
        }
        if (config.extrude_curb_amount != 0) {
            TriangleList<double>::extrude(
                *osm_triangle_lists.tl_street_curb[RoadType::STREET],
                {osm_triangle_lists.tl_street_curb[RoadType::STREET]},
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                config.extrude_curb_amount * config.scale,
                config.scale,
                1.f,
                config.uv_scales_street.at(RoadType::STREET),
                false,  // uvs_equal_lengths
                0.f);   // ambient_occlusion
            if (air_triangle_lists.tl_street_curb.contains(RoadType::PATH)) {
                TriangleList<double>::extrude(
                    *osm_triangle_lists.tl_street_curb[RoadType::PATH],
                    {osm_triangle_lists.tl_street_curb[RoadType::PATH]},
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr,
                    config.extrude_curb_amount * config.scale,
                    config.scale,
                    1.f,
                    config.uv_scales_street.at(RoadType::PATH),
                    false,  // uvs_equal_lengths
                    0.f);   // ambient_occlusion
            }
        }
        if (config.extrude_wall_amount != 0) {
            TriangleList<double>::extrude(
                *osm_triangle_lists.tl_street[RoadProperties{.type = RoadType::WALL, .nlanes = 1}].triangle_list,
                {osm_triangle_lists.tls_wall_wo_curb()},
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                config.extrude_wall_amount * config.scale,
                config.scale,
                1.f,
                config.uv_scale_highway_wall,
                false,  // uvs_equal_lengths
                0.f);   // ambient_occlusion
        }
        if (std::isnan(config.extrude_air_curb_amount)) {
            raise_streets(
                osm_triangle_lists.tls_street_wo_curb(),
                osm_triangle_lists.tls_wo_subtraction_and_water(),
                config.scale,
                config.raise_streets_amount);
        } else {
            raise_streets(
                TriangleList<double>::concatenated(
                    osm_triangle_lists.tls_street_wo_curb(),
                    air_triangle_lists.tls_street_wo_curb()),
                TriangleList<double>::concatenated(
                    osm_triangle_lists.tls_wo_subtraction_and_water(),
                    air_triangle_lists.tls_wo_subtraction_and_water()),
                config.scale,
                config.raise_streets_amount);
        }
        if (config.extrude_grass_amount != 0) {
            TriangleList<double>::extrude(
                *osm_triangle_lists.tl_terrain_extrusion[TerrainType::GRASS],
                {(*osm_triangle_lists.tl_terrain)[TerrainType::GRASS]},
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                config.extrude_grass_amount * config.scale,
                config.scale,
                1.f,
                config.uv_scale_grass,
                true,   // uvs_equal_lengths
                0.f);   // ambient_occlusion
        }
        if ((config.extrude_elevated_grass_amount != 0) &&
            osm_triangle_lists.tl_terrain->contains(TerrainType::ELEVATED_GRASS))
        {
            TriangleList<double>::extrude(
                *osm_triangle_lists.tl_terrain_extrusion[TerrainType::ELEVATED_GRASS_BASE],
                {(*osm_triangle_lists.tl_terrain)[TerrainType::ELEVATED_GRASS]},
                nullptr,
                nullptr,
                nullptr,
                nullptr,
                config.extrude_elevated_grass_amount * config.scale,
                config.scale,
                config.uv_scale_highway_wall,
                config.uv_scale_highway_wall,
                true,                                   // uvs_equal_lengths
                config.extrusion_ambient_occlusion);    // ambient_occlusion
        }
        if ((config.extrude_water_floor_amout != 0) &&
            osm_triangle_lists.tl_terrain->contains(TerrainType::WATER_FLOOR))
        {
            std::set<OrderableFixedArray<double, 3>> vertices_not_to_connect;
            for (const auto& p : map_outer_contour3) {
                vertices_not_to_connect.insert(OrderableFixedArray{ p });
            }
            TriangleList<double>::extrude(
                *osm_triangle_lists.tl_terrain_extrusion[TerrainType::WATER_FLOOR_BASE],
                {(*osm_triangle_lists.tl_terrain)[TerrainType::WATER_FLOOR]},
                nullptr,
                nullptr,
                nullptr,
                &vertices_not_to_connect,
                config.extrude_water_floor_amout * config.scale,
                config.scale,
                config.uv_scale_highway_wall,
                config.uv_scale_highway_wall,
                true,   // uvs_equal_lengths
                0.);    // ambient_occlusion
        }
    } catch (const EdgeException<double>& e) {
        handle_edge_exception(e, "Extrusion failed");
    } catch (const TriangleException<double>& e) {
        handle_triangle_exception(e, "Extrusion failed");
    }
    std::set<OrderableFixedArray<double, 3>> boundary_vertices;
    // Compute boundary vertices.
    if ((config.extrude_street_amount != 0) || (config.extrude_air_support_amount != 0))
    {
        LOG_INFO("compute vertices for street and air support extrusion");
        std::set<OrderableFixedArray<double, 3>> terrain_vertices;
        for (const auto& l : osm_triangle_lists.tl_terrain->map()) {
            for (const auto& t : l.second->triangles) {
                for (const auto& v : t.flat_iterable()) {
                    terrain_vertices.insert(OrderableFixedArray{v.position});
                }
            }
        }
        for (const auto& l : osm_triangle_lists.tls_street()) {
            for (const auto& t : l->triangles) {
                for (const auto& v : t.flat_iterable()) {
                    if (terrain_vertices.contains(OrderableFixedArray{v.position})) {
                        boundary_vertices.insert(OrderableFixedArray{v.position});
                    }
                }
            }
        }
    }
    if (config.extrude_street_amount != 0) {
        LOG_INFO("extrude streets");
        check_curb_validity(config.curb_alpha, config.curb2_alpha);
        if (!osm_triangle_lists.has_curb()) {  // "if (config.curb_alpha == 1)" not working for curbs from obj-models
            TriangleList<double>::extrude(
                *osm_triangle_lists.tl_terrain_extrusion[config.default_terrain_type],
                osm_triangle_lists.tls_street_wo_curb(),
                rvalue_address(osm_triangle_lists.tls_street_wo_curb_follower()),
                nullptr,
                nullptr,
                nullptr,
                config.extrude_street_amount * config.scale,
                config.scale,
                1,
                config.uv_scale_terrain,
                false,  // uvs_equal_lengths
                0.f);   // ambient_occlusion
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
                std::list<std::shared_ptr<TriangleList<double>>> source_triangles{triangle_lists.tls_curb_only()};
                TriangleList<double>::extrude(
                    *triangle_lists.tl_street_curb[RoadType::STREET],              // dest
                    triangle_lists.tls_street_wo_curb(),                           // triangle_lists
                    rvalue_address(triangle_lists.tls_street_wo_curb_follower()),  // follower_triangles
                    &source_triangles,                                             // source_triangles
                    &boundary_vertices,                                            // clamped_vertices
                    nullptr,                                                       // vertices_not_to_connect
                    config.extrude_street_amount * config.scale,                   // height
                    config.scale,                                                  // scale
                    1,                                                             // uv_scale_x
                    config.uv_scale_terrain,                                       // uv_scale_y
                    false,                                                         // uvs_equal_lengths
                    0.f);                                                          // ambient_occlusion
            };
            if (std::isnan(config.extrude_air_curb_amount)) {
                do_extrude(osm_triangle_lists);
            } else {
                do_extrude(osm_triangle_lists);
                do_extrude(air_triangle_lists);
            }
        }
    }

    std::unique_ptr<GroundBvh> ground_bvh;
    {
        LOG_INFO("compute ground bvh");
        auto ground_bvh_triangles = osm_triangle_lists.tls_smooth();
        if (config.with_terrain) {
            if (!config.base_osm_map_resource.empty()) {
                THROW_OR_ABORT("Terrain already set, cannot inherit from base OSM map");
            }
            ground_bvh = std::make_unique<GroundBvh>(ground_bvh_triangles);
        } else {
            if (config.base_osm_map_resource.empty()) {
                THROW_OR_ABORT("Base OSM map resource not set");
            }
            ground_bvh = std::make_unique<GroundBvh>(scene_node_resources.get_physics_arrays(config.base_osm_map_resource)->dcvas);
        }
        if (!config.racing_line_playback.empty()) {
            generate_racing_line_playback(
                config.racing_line_track,
                config.racing_line_playback,
                normalization_matrix_,
                get_geographic_mapping(TransformationMatrix<double, double, 3>::identity()),
                *ground_bvh);
        }
        LOG_INFO("add_models_to_model_nodes");
        try {
            add_models_to_model_nodes(
                *hri_.bri,
                *ground_bvh,
                scene_node_resources,
                nodes,
                ways,
                config.scale,
                config.game_level);
        } catch (const TriangleException<double>& e) {
            if (auto prefix = try_getenv("EXCEPT_MESH_AROUND_PREFIX"); prefix.has_value()) {
                auto coords = (e.a + e.b + e.c) / 3.;
                {
                    FixedArray<double, 3> pos{ coords(0), coords(1), 0.f };
                    auto m = get_geographic_mapping(TransformationMatrix<double, double, 3>::identity());
                    std::cerr.precision(15);
                    std::cerr << "Saving mesh around " << pos << " | " << m.transform(pos) << std::endl;
                }
                for (float r : string_to_vector(getenv_default("EXCEPT_MESH_AROUND_RADIUSES", "0.05 0.2 0.5"), safe_stof)) {
                    plot_mesh(
                        ArrayShape{ 2000, 2000 },         // image_size
                        1,                                // line_thickness
                        4,                                // point_size
                        get_triangles_around(             // triangles
                            ground_bvh_triangles,
                            { coords(0), coords(1) },
                            r),
                        {},                               // contour
                        {                                 // highlighted_nodes
                            {e.a(0), e.a(1), 0.f},
                            {e.b(0), e.b(1), 0.f},
                            {e.c(0), e.c(1), 0.f}
                        },
                        {}                                // crossed_nodes
                    ).T().reversed(0).save_to_file(prefix.value() + "_r_" + std::to_string(r) + ".png");
                }
                handle_triangle_exception(e, "add models failed, debug image saved");
            } else {
                handle_triangle_exception(e, "add models failed, consider setting the 'EXCEPT_MESH_AROUND_PREFIX' environment variable");
            }
        }

        if (config.with_tree_nodes && !config.tree_resource_names.empty()) {
            ResourceNameCycle rnc{config.tree_resource_names};
            LOG_INFO("add_trees_to_tree_nodes");
            add_trees_to_tree_nodes(
                *hri_.bri,
                // steiner_points,
                rnc,
                config.min_dist_to_road,
                all_holes_bvh,
                *ground_bvh,
                nodes,
                config.scale);
        }

        if (config.forest_outline_tree_distance != INFINITY && !config.tree_resource_names.empty()) {
            ResourceNameCycle rnc{config.tree_resource_names};
            LOG_INFO("add_trees_to_forest_outlines");
            add_trees_to_forest_outlines(
                *hri_.bri,
                // steiner_points,
                rnc,
                config.min_dist_to_road,
                all_holes_bvh,
                *ground_bvh,
                nodes,
                ways,
                config.forest_outline_tree_distance,
                config.forest_outline_tree_inwards_distance,
                config.scale);
            // add_binary_vegetation(
            //     tls,
            //     Material{
            //         .texture: grass_texture,
            //         .mixed_texture: "",
            //         .overlap_npixels: 0,
            //         .blend_mode: BlendMode::BINARY,
            //         .wrap_mode: WrapMode::CLAMP_TO_EDGE,
            //         .collide: false,
            //         .aggregate_mode: AggregateMode::ONCE,
            //         .emissivity = OrderableFixedArray{DEFAULT_EMISSIVITY * config.emissivity_factor},
            //         .ambience = OrderableFixedArray{DEFAULT_AMBIENCE * config.ambience_factor},
            //         .diffusivity = OrderableFixedArray{DEFAULT_DIFFUSIVITY * config.diffusivity_factor},
            //         .specularity = OrderableFixedArray{DEFAULT_SPECULARITY * config.specularity_factor},},
            //     grass_texture,
            //     tree_texture,
            //     tree_texture_2,
            //     *tl_terrain,
            //     scale);
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
        for (auto& t : air_or_osm.tl_tunnel_crossing->triangles) {
            for (auto& v : t.flat_iterable()) {
                v.position(2) += config.default_tunnel_pipe_height * config.scale;
            }
        }

        // save_obj("/tmp/tl_terrain0.obj", IndexedFaceSet<float, size_t>{tl_terrain_->triangles_});
        std::set<const FixedArray<ColoredVertex<double>, 3>*> triangles_to_delete;
        for (const auto& t : air_or_osm.tl_air_support->triangles) {
            if (boundary_vertices.contains(OrderableFixedArray{t(0).position}) ||
                boundary_vertices.contains(OrderableFixedArray{t(1).position}) ||
                boundary_vertices.contains(OrderableFixedArray{t(2).position}))
            {
                triangles_to_delete.insert(&t);
            }
        }
        if (config.extrude_air_support_amount != 0) {
            TriangleList<double>::extrude(
                *air_or_osm.tl_air_support,                         // dest
                {air_or_osm.tl_air_support},                        // triangle_lists
                nullptr,                                            // follower_triangles
                nullptr,                                            // source_triangles
                &boundary_vertices,                                 // clamped_vertices
                nullptr,                                            // vertices_not_to_connect
                config.extrude_air_support_amount * config.scale,   // height
                config.scale,                                       // scale
                1,                                                  // uv_scale_x
                config.uv_scale_terrain,                            // uv_scale_y
                false,                                              // uvs_equal_lengths
                0.f);                                               // ambient_occlusion
            air_or_osm.tl_air_support->triangles.remove_if([&triangles_to_delete](const FixedArray<ColoredVertex<double>, 3>& t){
                return triangles_to_delete.contains(&t);
            });
        }
    }

    // for (auto& l : tls_ground) {
    //     colorize_height_map(l->triangles_);
    // }

    if (!config.grass_resource_names.empty()) {
        ResourceNameCycle rnc{ config.grass_resource_names };
        LOG_INFO("add_grass_inside_triangles");
        add_grass_inside_triangles(
            *hri_.bri,
            rnc,
            *(*tl_terrain_)[TerrainType::GRASS],
            config.scale,
            config.much_grass_distance);
    }
    LOG_INFO("calculate spawn points");
    calculate_spawn_points(
        spawn_points_,
        street_rectangles,
        config.scale,
        config.driving_direction);
    // if (false) {
    //     resource_instance_positions_.clear();
    //     for (const auto& p : spawn_points_) {
    //         resource_instance_positions_["grass12y"].push_back(ResourceInstanceDescriptor{.position = p.position});
    //     }
    //     for (const auto& p : osm_triangle_lists.tls_street()) {
    //         for (const auto& t : p->triangles_) {
    //             for (const auto& v : t.flat_iterable()) {
    //                 auto it = node_height_bindings.find(OrderableFixedArray<float, 2>{v.position(0), v.position(1)});
    //                 if (it != node_height_bindings.end()) {
    //                     if (it->second == "1792772911") {  // node 1792772911 4002287619
    //                         resource_instance_positions_["grass12y"].push_back(ResourceInstanceDescriptor{.position = v.position});
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }

    tls_no_grass_ = osm_triangle_lists.tls_no_grass();

    LOG_INFO("calculate normals");
    // Normals are invalid after "apply_heightmap"
    for (auto& l2 : osm_triangle_lists.tls_wo_subtraction_and_water()) {
        l2->calculate_triangle_normals();
    }

    // save_obj("/tmp/tl_terrain_final.obj", IndexedFaceSet<float, size_t>{osm_triangle_lists.tl_terrain->triangles_});
    // save_obj("/tmp/tl_tunnel_pipe_final.obj", IndexedFaceSet<float, size_t>{osm_triangle_lists.tl_tunnel_pipe->triangles_});
    // save_obj("/tmp/tl_street_final.obj", IndexedFaceSet<float, size_t>{osm_triangle_lists.tl_street->triangles_});

    if (!config.street_bumps_central_resource_names.empty() ||
        !config.street_bumps_endpoint0_resource_names.empty() ||
        !config.street_bumps_endpoint1_resource_names.empty())
    {
        LOG_INFO("draw bumps");
        draw_into_street_rectangles(osm_triangle_lists.tl_street, street_rectangles, scene_node_resources, config.bump_height, config.scale);
    }

    if (config.with_terrain) {
        for (const auto& [road_type, blend] : config.blend_street) {
            if (blend) {
                draw_terrain_triangles(
                    *osm_triangle_lists.tl_terrain_visuals[config.default_terrain_type],
                    osm_triangle_lists.street_triangles(road_type));
            }
        }
    }

    if (!std::isnan(config.extrude_air_curb_amount)) {
        LOG_INFO("inser air triangles lists");
        // If "extrude_air_curb_amount" is NOT NAN,
        // insert the air triangle lists here.
        for (auto& l : air_triangle_lists.tl_street_curb.map()) {
            air_triangle_lists.tl_air_street_curb[l.first]->triangles = std::move(l.second->triangles);
        }
        osm_triangle_lists.insert(air_triangle_lists);
    }

    TriangleList<double>::convert_triangle_to_vertex_normals(osm_triangle_lists.tls_with_vertex_normals());
    TriangleList<double>::ambient_occlusion_by_curvature(osm_triangle_lists.tls_with_vertex_normals(), config.laplace_ambient_occlusion);
    TriangleList<double>::convert_triangle_to_vertex_normals(tls_wall_barriers);

    std::list<std::shared_ptr<TriangleList<double>>> tls_all;
    if (!config.water_texture.empty()) {
        std::list<std::pair<WaterType, std::list<FixedArray<double, 2>>>> water_contours =
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
                1.f,                    // uv_period
                config.water_height,
                terrain_color,
                getenv_default("WATER_CONTOUR_TRIANGLES_FILENAME", ""),
                getenv_default("WATER_CONTOUR_FILENAME", ""),
                getenv_default("WATER_TRIANGLE_FILENAME", ""),
                WaterType::UNDEFINED,
                WaterType::UNDEFINED);
        } catch (const p2t::PointException& e) {
            handle_point_exception(e, "Could not triangulate water (WATER_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const EdgeException<double>& e) {
            handle_edge_exception(e, "Could not triangulate water (WATER_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const TriangleException<double>& e) {
            handle_triangle_exception(e, "Could not triangulate water (WATER_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        }
        tls_all = osm_triangle_lists.tls_wo_subtraction_w_water();
    } else {
        tls_all = osm_triangle_lists.tls_wo_subtraction_and_water();
    }
    for (auto& l : std::list<const std::list<std::shared_ptr<TriangleList<double>>>*>{
            &tls_all,
            &tls_buildings,
            &tls_wall_barriers})
    {
        for (auto& l2 : *l) {
            if (!l2->triangles.empty()) {
                auto cva = l2->triangle_array();
                modulo_uv(*cva);
                hri_.acvas->dcvas.push_back(cva);
            }
        }
    }
    {
        LOG_INFO("calculate spawn points");
        FacadeTextureCycle ftc({});
        std::list<Building> spawn_lines = get_buildings_or_wall_barriers(
            BuildingType::SPAWN_LINE,
            ways,
            0,  // building_bottom
            0,  // default_building_top
            NAN,
            {},
            ftc);
        try {
            for (const Building& bu : spawn_lines) {
                auto iteam = bu.way.tags.find("team");
                for (auto it = bu.way.nd.begin(); it != bu.way.nd.end(); ++it) {
                    auto next = it;
                    ++next;
                    if (next != bu.way.nd.end()) {
                        FixedArray<double, 2> p = (nodes.at(*it).position + nodes.at(*next).position) / 2.;
                        FixedArray<double, 2> dir = nodes.at(*it).position - nodes.at(*next).position;
                        double len2 = sum(squared(dir));
                        if (len2 < 1e-12) {
                            throw PointException{ p, "Spawn direction too small" };
                        }
                        dir /= std::sqrt(len2);
                        double height;
                        if (!ground_bvh->height(height, p)) {
                            throw PointException{ p, "Spawn line out of bounds" };
                        }
                        spawn_points_.push_back(SpawnPoint{
                            .type = SpawnPointType::SPAWN_LINE,
                            .location = WayPointLocation::UNKNOWN,
                            .position = {p(0), p(1), height},
                            .rotation = {0.f, 0.f, (float)std::atan2(dir(0), -dir(1))},
                            .team = (iteam == bu.way.tags.end()) ? "" : iteam->second});
                    }
                }
            }
        } catch (const PointException<double, 2>& p) {
            handle_point_exception2(p, "Bould not apply height map to spawn lines");
        }
    }
    auto split_grass = [this, &ground_street_bvh](
        TerrainType source_terrain_type,
        TerrainType target_terrain_type,
        const TerrainStyleDistancesToBdry& target_terrain_distances_to_bdry)
    {
        if (target_terrain_distances_to_bdry.is_active) {
            if (auto tit = tl_terrain_->map().find(source_terrain_type); tit != tl_terrain_->map().end())
            {
                LOG_INFO(
                    "extract " + terrain_type_to_string(target_terrain_type) +
                    " from " + terrain_type_to_string(source_terrain_type));
                double max_dist = target_terrain_distances_to_bdry.max_distance_to_bdry * scale_;
                tl_terrain_->insert(target_terrain_type, std::make_shared<TriangleList<double>>(
                    terrain_type_to_string(target_terrain_type) + "_autogen",
                    tit->second->material,
                    tit->second->physics_material));
                auto& wayside_grass = *(*tl_terrain_)[target_terrain_type];
                tit->second->triangles.remove_if([&ground_street_bvh, &max_dist, &wayside_grass](const FixedArray<ColoredVertex<double>, 3>& tri){
                    for (const auto& v : tri.flat_iterable()) {
                        if (ground_street_bvh.has_neighbor(
                            FixedArray<double, 2>{
                                v.position(0),
                                v.position(1)},
                            max_dist))
                        {
                            wayside_grass.triangles.push_back(tri);
                            return true;
                        }
                    }
                    return false;
                });
            }
        }
    };
    // Extract wayside2_grass triangles from grass triangles
    split_grass(TerrainType::GRASS, TerrainType::WAYSIDE2_GRASS, terrain_styles_.near_wayside2_grass_terrain_style.distances_to_bdry());
    // Extract wayside1_grass triangles from wayside2_grass triangles
    split_grass(TerrainType::WAYSIDE2_GRASS, TerrainType::WAYSIDE1_GRASS, terrain_styles_.near_wayside1_grass_terrain_style.distances_to_bdry());
    {
        CollidableTriangleSampler cts{terrain_styles_, scale_, UpAxis::Z};
        LOG_INFO("add near hitboxes");
        cts.add_near_hitboxes(terrain_triangles(), street_bvh(), hri_);
        LOG_INFO("add far instances");
        cts.add_far_hitboxes(terrain_triangles(), street_bvh(), hri_);
    }
    LOG_INFO("save obj files if requested");
    save_to_obj_file_if_requested(debug_prefix);
    save_bad_triangles_to_obj_file_if_requested(debug_prefix);
    {
        LOG_INFO("calculate waypoints");
        std::list<TerrainWayPoints> terrain_way_point_lines = get_terrain_way_points(ways);
        try {
            if (config.with_street_way_points) {
                calculate_waypoint_adjacency(
                    way_points_[WayPointLocation::STREET],
                    {},
                    way_point_edge_descriptors[WayPointLocation::STREET],
                    nodes,
                    *ground_bvh,
                    nullptr,        // to_meters
                    nullptr,        // sample_solo_mesh
                    config.scale);
            }
            if (config.with_sidewalk_way_points) {
                calculate_waypoint_adjacency(
                    way_points_[WayPointLocation::SIDEWALK],
                    {},
                    way_point_edge_descriptors[WayPointLocation::SIDEWALK],
                    nodes,
                    *ground_bvh,
                    nullptr,        // to_meters
                    nullptr,        // sample_solo_mesh
                    config.scale);
            }
            if (!terrain_way_point_lines.empty() ||
                !way_point_edge_descriptors[WayPointLocation::EXPLICIT].empty())
            {
                const auto& navigation_dcvas = config.navmesh_resource.empty()
                    ? get_physics_arrays()->dcvas
                    : scene_node_resources.get_physics_arrays(config.navmesh_resource)->dcvas;
                if (!config.refine_explicit_waypoints) {
                    calculate_waypoint_adjacency(
                        way_points_[WayPointLocation::EXPLICIT],
                        terrain_way_point_lines,
                        way_point_edge_descriptors[WayPointLocation::EXPLICIT],
                        nodes,
                        *ground_bvh,
                        nullptr,        // to_meters
                        nullptr,        // sample_solo_mesh
                        config.scale);
                } else {
                    // Apply the inverse rotation that is applied to the hitboxes,
                    // and divide by scale as opposed to multiplying.
                    auto rotation = tait_bryan_angles_2_matrix<float>({ -90.f * degrees, 0.f, 0.f });
                    std::list<FixedArray<ColoredVertex<float>, 3>> triangles;
                    for (const auto& cvas : navigation_dcvas) {
                        for (const auto& t : cvas->triangles) {
                            triangles.push_back(FixedArray<ColoredVertex<float>, 3>{
                                t(0).casted<float>().rotated(rotation).scaled(1.f / (float)scale_),
                                t(1).casted<float>().rotated(rotation).scaled(1.f / (float)scale_),
                                t(2).casted<float>().rotated(rotation).scaled(1.f / (float)scale_)});
                        }
                    }
                    IndexedFaceSet<float, float, size_t> indexed_face_set{ triangles };
                    // save_obj("/tmp/asd.obj", indexed_face_set, nullptr);
                    NavigationMeshBuilder nmb{
                        indexed_face_set,
                        NavigationMeshConfig{
                            .cell_size = 1.f,
                            .agent_radius = config.agent_radius}};
                    calculate_waypoint_adjacency(
                        way_points_[WayPointLocation::EXPLICIT],
                        terrain_way_point_lines,
                        way_point_edge_descriptors[WayPointLocation::EXPLICIT],
                        nodes,
                        *ground_bvh,
                        rvalue_address(rotation.casted<double>() / scale_),
                        &nmb.ssm,
                        config.scale);
                }
            }
        } catch (const PointException<double, 2>& e) {
            handle_point_exception2(e, "Could not calculate waypoint adjacency");
        } catch (const PointException<double, 3>& e) {
            handle_point_exception3(e, "Could not calculate waypoint adjacency");
        } catch (const TriangleException<double>& e) {
            handle_triangle_exception(e, "Could not calculate waypoint adjacency");
        } catch (const EdgeException<double>& e) {
            handle_edge_exception(e, "Could not calculate waypoint adjacency");
        }
    }
    LOG_INFO("print waypoints if requested");
    print_waypoints_if_requested(debug_prefix);
}

OsmMapResource::OsmMapResource(
    SceneNodeResources& scene_node_resources,
    const std::string& level_filename,
    const std::string& debug_prefix)
: hri_{ scene_node_resources, { NAN, NAN, NAN }, NAN },
  scene_node_resources_{ scene_node_resources },
  terrain_styles_{}
{
    auto ifstr_p = create_ifstream(level_filename, std::ios::binary);
    auto& ifstr = *ifstr_p;
    if (ifstr.fail()) {
        THROW_OR_ABORT("Could not open input OSM-map binary file \"" + level_filename + '"');
    }
    cereal::BinaryInputArchive iarchive(ifstr);
    iarchive(*this);
    if (ifstr.fail()) {
        THROW_OR_ABORT("Could not read from file \"" + level_filename + '"');
    }
    print_waypoints_if_requested(debug_prefix);
    save_to_obj_file_if_requested(debug_prefix);
    save_bad_triangles_to_obj_file_if_requested(debug_prefix);
}

const Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>& OsmMapResource::street_bvh() const {
    {
        std::shared_lock lock{street_bvh_mutex_};
        if (street_bvh_ != nullptr) {
            return *street_bvh_;
        }
    }
    if (street_bvh_ == nullptr) {
        std::scoped_lock lock{street_bvh_mutex_};
        street_bvh_.reset(new Bvh<double, FixedArray<FixedArray<double, 3>, 3>, 3>{{0.1, 0.1, 0.1}, 10});
        for (const auto& lst : tls_no_grass_) {
            for (const auto& t : lst->triangles) {
                FixedArray<FixedArray<double, 3>, 3> tri{
                    t(0).position,
                    t(1).position,
                    t(2).position};
                street_bvh_->insert(tri, tri);
            }
        }
    }
    return *street_bvh_;
}

void OsmMapResource::save_to_file(const std::string& filename) const {
    auto ofstr = create_ofstream(filename, std::ios::binary);
    if (ofstr->fail()) {
        THROW_OR_ABORT("Could not open output OSM-map binary file \"" + filename + '"');
    }
    cereal::BinaryOutputArchive oarchive(*ofstr);
    oarchive(*this);
    ofstr->flush();
    if (ofstr->fail()) {
        THROW_OR_ABORT("Could not write to file \"" + filename + '"');
    }
}

void OsmMapResource::save_to_obj_file(
    const std::string& prefix,
    const TransformationMatrix<float, double, 3>& tm) const
{
    auto filename = prefix + "_osm_map.obj";
    auto primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
    std::map<TextureDescriptor, std::string> autogen_textures;
    auto get_filename = [&](const TextureDescriptor& desc){
        auto it = autogen_textures.find(desc);
        if (it == autogen_textures.end()) {
            std::string result = primary_rendering_resources->get_texture_filename(
                desc,
                filename + ".tex." + std::to_string(autogen_textures.size()) + ".png");
            autogen_textures.insert({ desc, result });
            return result;
        } else {
            return it->second;
        }
    };
    std::list<std::shared_ptr<ColoredVertexArray<double>>> mdcvas;
    for (const auto& l : hri_.acvas->dcvas) {
        mdcvas.push_back(l->transformed<double>(tm, ""));
    }
    save_obj(
        filename,
        mdcvas,  // get_physics_arrays()->cvas
        [&](const Material& m){
            ObjMaterial result{
                .ambience = m.ambience,
                .diffusivity = m.diffusivity,
                .specularity = m.specularity};
            if (!m.textures.empty()) {
                const auto& desc = m.textures[0].texture_descriptor;
                result.color_texture = get_filename(desc);
                result.bump_texture = get_filename(TextureDescriptor{
                    .color = {.filename = desc.normal.filename, .average = desc.normal.average},
                    .color_mode = ColorMode::RGB,
                    .anisotropic_filtering_level = desc.anisotropic_filtering_level});
                result.has_alpha_texture = (desc.color_mode == ColorMode::RGBA);
            }
            return result;
        });
}

void OsmMapResource::save_bad_triangles_to_obj_file(const std::string& filename) const {
    TriangleList<double> bad_triangles{
        "bad_trinalges",
        Material{
            .ambience = {1.f, 0.f, 0.f},
            .diffusivity = {1.f, 0.f, 0.f},
            .specularity = {1.f, 0.f, 0.f}},
        PhysicsMaterial::NONE};
    for (const auto& l : hri_.acvas->dcvas) {
        for (const auto& t : l->triangles) {
            auto tlc = triangle_largest_cosine<double, 3>({
                t(0).position,
                t(1).position,
                t(2).position});
            if (std::isnan(tlc) || (tlc > 0.999))
            {
                bad_triangles.triangles.push_back(t);
            }
        }
    }
    save_obj(
        filename,
        {bad_triangles.triangle_array()},
        [&](const Material& m){
            return ObjMaterial{
                .ambience = m.ambience,
                .diffusivity = m.diffusivity,
                .specularity = m.specularity};
        });
}

OsmMapResource::~OsmMapResource()
{}

void OsmMapResource::preload(const RenderableResourceFilter& filter) const {
    hri_.preload(filter);
}

TerrainTriangles OsmMapResource::terrain_triangles() const {
    return TerrainTriangles{
        .grass = tl_terrain_->contains(TerrainType::GRASS) ? &(*tl_terrain_)[TerrainType::GRASS]->triangles : nullptr,
        .elevated_grass = tl_terrain_->contains(TerrainType::ELEVATED_GRASS) ? &(*tl_terrain_)[TerrainType::ELEVATED_GRASS]->triangles : nullptr,
        .wayside1_grass = tl_terrain_->contains(TerrainType::WAYSIDE1_GRASS) ? &(*tl_terrain_)[TerrainType::WAYSIDE1_GRASS]->triangles : nullptr,
        .wayside2_grass = tl_terrain_->contains(TerrainType::WAYSIDE2_GRASS) ? &(*tl_terrain_)[TerrainType::WAYSIDE2_GRASS]->triangles : nullptr,
        .flowers = tl_terrain_->contains(TerrainType::FLOWERS) ? &(*tl_terrain_)[TerrainType::FLOWERS]->triangles : nullptr,
        .trees = tl_terrain_->contains(TerrainType::TREES) ? &(*tl_terrain_)[TerrainType::TREES]->triangles : nullptr
    };
}

std::list<const std::list<FixedArray<ColoredVertex<double>, 3>>*> OsmMapResource::no_grass() const {
    std::list<const std::list<FixedArray<ColoredVertex<double>, 3>>*> result;
    for (const auto& lst : tls_no_grass_) {
        result.push_back(&lst->triangles);
    }
    return result;
}

void OsmMapResource::instantiate_renderable(const InstantiationOptions& options) const
{
    hri_.instantiate_renderable(options);
    if (terrain_styles_.requires_renderer()) {
        options.scene_node->add_renderable("osm_map_near", std::make_shared<RenderableTriangleSampler>(
            scene_node_resources_,
            terrain_styles_,
            terrain_triangles(),
            no_grass(),
            &street_bvh(),
            scale_,
            UpAxis::Z));
    }
    // if (rbvh_ == nullptr) {
    //     rbvh_ = std::make_shared<BvhResource>(cvas_);
    // }
    // rbvh_->instantiate_renderable(options);
}

std::shared_ptr<AnimatedColoredVertexArrays> OsmMapResource::get_physics_arrays() const {
    return hri_.get_physics_arrays();
}

std::shared_ptr<ISceneNodeResource> OsmMapResource::generate_grind_lines(
    float edge_angle,
    float averaged_normal_angle,
    const ColoredVertexArrayFilter& filter) const
{
    return std::make_shared<ColoredVertexArrayResource>(get_physics_arrays())->generate_grind_lines(
        edge_angle,
        averaged_normal_angle,
        filter);
}

void OsmMapResource::modify_physics_material_tags(
    PhysicsMaterial add,
    PhysicsMaterial remove,
    const ColoredVertexArrayFilter& filter)
{
    hri_.modify_physics_material_tags(add, remove, filter);
}

void OsmMapResource::create_barrier_triangle_hitboxes(
    float depth,
    PhysicsMaterial destination_physics_material,
    const ColoredVertexArrayFilter& filter)
{
    try {
        hri_.create_barrier_triangle_hitboxes(depth, destination_physics_material, filter);
    } catch (const TriangleException<double>& e) {
        handle_triangle_exception(e, "Could not decompose terrain into convex regions");
    }
}

TransformationMatrix<double, double, 3> OsmMapResource::get_geographic_mapping(
    const TransformationMatrix<double, double, 3>& absolute_model_matrix) const
{
    return get_geographic_mapping_3d(normalization_matrix_, absolute_model_matrix, (double)scale_);
}

std::list<SpawnPoint> OsmMapResource::spawn_points() const {
    return spawn_points_;
}

std::map<WayPointLocation, PointsAndAdjacency<double, 3>> OsmMapResource::way_points() const {
    return way_points_;
}

void OsmMapResource::print(std::ostream& ostr) const {
    ostr << "OsmMapResource\n";
    for (const auto& cva : hri_.acvas->scvas) {
        cva->print(ostr);
    }
    for (const auto& cva : hri_.acvas->dcvas) {
        cva->print(ostr);
    }
}

void plot_way_points_and_obstacles(
    const std::string& filename,
    const PointsAndAdjacency<double, 3>& pa,
    const std::list<FixedArray<double, 2>>& bounding_contour,
    const std::list<FixedArray<double, 3>>& hitbox_positions)
{
    std::list<FixedArray<FixedArray<double, 2>, 3>> triangles;
    std::list<FixedArray<FixedArray<double, 2>, 2>> edges;
    std::list<std::list<FixedArray<double, 2>>> contours;
    std::list<FixedArray<double, 2>> highlighted_nodes;
    for (size_t c = 0; c < pa.adjacency.shape(1); ++c) {
        for (const auto& r : pa.adjacency.column(c)) {
            if (r.first != c) {
                edges.push_back(FixedArray<FixedArray<double, 2>, 2>{
                    FixedArray<double, 2>{pa.points.at(c)(0), pa.points.at(c)(1)},
                    FixedArray<double, 2>{pa.points.at(r.first)(0), pa.points.at(r.first)(1)}});
            }
        }
    }
    contours.push_back(bounding_contour);
    for (const auto& p : hitbox_positions) {
        highlighted_nodes.push_back(FixedArray<double, 2>{p(0), p(1)});
    }
    if (!edges.empty() || !highlighted_nodes.empty()) {
        plot_mesh_svg(filename, 600.f, 600.f, triangles, edges, contours, highlighted_nodes);
    }
}

void OsmMapResource::print_waypoints_if_requested(const std::string& debug_prefix) const {
    if (auto wf = try_getenv("OSM_WAYPOINT_PREFIX"); wf.has_value()) {
        auto rs = try_getenv("OSM_WAYPOINT_BBOX_RADIUS");
        if (!rs.has_value()) {
            THROW_OR_ABORT("Please specify the \"OSM_WAYPOINT_BBOX_RADIUS\" environment variable (should be in the range 1 - 2)");
        }
        double r = safe_stod(rs.value());
        // way_points_.at(WayPointLocation::STREET).plot(wf + debug_prefix + "street.svg", 600, 600, 0.1f);
        // way_points_.at(WayPointLocation::SIDEWALK).plot(wf + debug_prefix + "sidewalk.svg", 600, 600, 0.1f);
        // way_points_.at(WayPointLocation::EXPLICIT).plot(wf + debug_prefix + "explicit.svg", 600, 600, 0.1f);

        std::list<FixedArray<double, 2>> bounding_contour{
            FixedArray<double, 2>{-r, -r},
            FixedArray<double, 2>{+r, -r},
            FixedArray<double, 2>{+r, +r},
            FixedArray<double, 2>{-r, +r},
            FixedArray<double, 2>{-r, -r}};
        auto hitbox_positions = hri_.bri->hitbox_positions();
        if (way_points_.contains(WayPointLocation::STREET)) {
            plot_way_points_and_obstacles(wf.value() + debug_prefix + "street.svg", way_points_.at(WayPointLocation::STREET), bounding_contour, hitbox_positions);
        }
        if (way_points_.contains(WayPointLocation::SIDEWALK)) {
            plot_way_points_and_obstacles(wf.value() + debug_prefix + "sidewalk.svg", way_points_.at(WayPointLocation::SIDEWALK), bounding_contour, hitbox_positions);
        }
        if (way_points_.contains(WayPointLocation::EXPLICIT)) {
            plot_way_points_and_obstacles(wf.value() + debug_prefix + "explicit.svg", way_points_.at(WayPointLocation::EXPLICIT), bounding_contour, hitbox_positions);
        }
    }
}

void OsmMapResource::save_to_obj_file_if_requested(const std::string& debug_prefix) const
{
    if (auto wp = try_getenv("OSM_OBJ_PREFIX"); wp.has_value()) {
        save_to_obj_file(wp.value() + debug_prefix + ".obj", TransformationMatrix<float, double, 3>::identity());
    }
}

void OsmMapResource::save_bad_triangles_to_obj_file_if_requested(const std::string& debug_prefix) const {
    if (auto wp = try_getenv("OSM_BAD_OBJ_PREFIX"); wp.has_value()) {
        save_bad_triangles_to_obj_file(wp.value() + debug_prefix + ".obj");
    }
}

void OsmMapResource::handle_point_exception3(
    const PointException<double, 3>& e,
    const std::string& message) const
{
    auto m = get_geographic_mapping(TransformationMatrix<double, double, 3>::identity());
    throw std::runtime_error(e.str(message, &m));
}

void OsmMapResource::handle_point_exception2(
    const PointException<double, 2>& e,
    const std::string& message) const
{
    handle_point_exception3(PointException<double, 3>{FixedArray<double, 3>{ e.point(0), e.point(1), 0. }, e.what()}, message);
}

void OsmMapResource::handle_point_exception(
    const p2t::PointException& e,
    const std::string& message) const
{
    FixedArray<double, 3> pos{e.point.x, e.point.y, 0.};
    auto m = get_geographic_mapping(TransformationMatrix<double, double, 3>::identity());
    std::stringstream sstr;
    sstr.precision(15);
    sstr << message << " at position " << m.transform(pos) << ": " << e.what() << std::endl;
    throw std::runtime_error(sstr.str());
}

void OsmMapResource::handle_edge_exception(
    const EdgeException<double>& e,
    const std::string& message) const
{
    auto m = get_geographic_mapping(TransformationMatrix<double, double, 3>::identity());
    throw std::runtime_error(e.str(message, &m));
}

void OsmMapResource::handle_triangle_exception(
    const TriangleException<double>& e,
    const std::string& message) const
{
    auto m = get_geographic_mapping(TransformationMatrix<double, double, 3>::identity());
    throw std::runtime_error(e.str(message, &m));
}
