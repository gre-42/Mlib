#include "Osm_Map_Resource.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Geography/Geographic_Coordinates.hpp>
#include <Mlib/Geometry/Coordinates/Normalized_Points_Fixed.hpp>
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>
#include <Mlib/Geometry/Exceptions/Point_Exception.hpp>
#include <Mlib/Geometry/Exceptions/Triangle_Exception.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Geometry/Mesh/Bone.hpp>
#include <Mlib/Geometry/Mesh/Cleanup/Modulo_Uv.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Plot.hpp>
#include <Mlib/Geometry/Mesh/Point_And_Flags.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Geometry/Mesh/Save_Obj.hpp>
#include <Mlib/Geometry/Mesh/Terrain_Uv.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Largest_Cosine.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Mesh/Triangles_Around.hpp>
#include <Mlib/Geometry/Mesh/Up_Axis.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/StbImage1.hpp>
#include <Mlib/Images/StbImage3.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Transformation/Translation_Matrix.hpp>
#include <Mlib/Navigation/NavigationMeshBuilder.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Add_Grass_Inside_Triangles.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Add_Grass_on_Steiner_Points.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Add_Models_To_Model_Nodes.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Add_Street_Steiner_Points.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Add_Trees_To_Forest_Outlines.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Add_Trees_To_Tree_Nodes.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Add_Trees_To_Zonemap.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Apply_Displacement_Map.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Apply_Heightmap_And_Smoothen.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Bounding_Info.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Building.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Calculate_Spawn_Points.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Calculate_Waypoint_Adjacency.hpp>
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
#include <Mlib/Osm_Loader/Osm_Map_Resource/Draw_Waysides.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Facade_Texture_Cycle.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Generate_Racing_Line_Playback.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Get_Buildings_Or_Wall_Barriers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Get_Map_Outer_Contour.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Get_Morphology.hpp>
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
#include <Mlib/Render/Resource_Managers/Rendering_Resources.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Descriptors/Object_Resource_Descriptor.hpp>
#include <Mlib/Scene_Graph/Descriptors/Resource_Instance_Descriptor.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Instantiation/Child_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Instantiation/Root_Instantiation_Options.hpp>
#include <Mlib/Scene_Graph/Joined_Way_Point_Sandbox.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Spawn_Point.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>
#include <Mlib/Scene_Graph/Way_Point_Sandbox.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Strings/String_To_Scene_Pos.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Threads/Thread_Top.hpp>
#include <cereal/access.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>
#include <mutex>
#include <poly2tri/edge_exception.hpp>
#include <poly2tri/point_exception.hpp>
#include <stb_cpp/stb_array.hpp>
#include <stb_cpp/stb_image_load.hpp>

// #undef LOG_FUNCTION
// #undef LOG_INFO
// #define LOG_FUNCTION(msg) ::Mlib::Log log(msg, "LOG_OSM_MAP")
// #define LOG_INFO(msg) log.info(msg)

using namespace Mlib;

OsmMapResource::OsmMapResource(
    SceneNodeResources& scene_node_resources,
    const OsmResourceConfig& config,
    const std::string& debug_prefix,
    FileStorageType file_storage_type)
    : hri_{ scene_node_resources, { 90.f * degrees, 0.f, 0.f }, config.scale }
    , scene_node_resources_{ scene_node_resources }
    , scale_{ config.scale }
    , normalization_matrix_{ uninitialized }
    , triangulation_normalization_matrix_{ uninitialized }
    , terrain_styles_{ config.triangle_sampler_resource_config }
{
    LOG_FUNCTION("OsmMapResource::OsmMapResource");
    NodesAndWays naws_or;
    NormalizedPointsFixed<double> normalized_points{ ScaleMode::NONE, OffsetMode::CENTERED };

    FunctionGuard fg{ "OSM map resource" };

    fg.update("Parse OSM XML");
    parse_osm_xml(
        config.filename,
        config.scale,
        normalized_points,
        normalization_matrix_,
        naws_or.nodes,
        naws_or.ways);
    triangulation_normalization_matrix_ = normalization_matrix_.pre_scaled(config.triangulation_scale);
    
    fg.update("Smoothen ways");
    NodesAndWays naws_smooth = smoothen_ways(
        naws_or,
        config.smoothed_highways,
        config.smoothed_aeroways,
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
    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> tls_buildings;
    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> tls_wall_barriers;
    std::map<OrderableFixedArray<CompressedScenePos, 2>, NodeHeightBinding> node_height_bindings;
    std::unordered_map<FixedArray<CompressedScenePos, 3>*, VertexHeightBinding<CompressedScenePos>> vertex_height_bindings;
    std::list<SteinerPointInfo> steiner_points;
    std::list<StreetRectangle> street_rectangles;
    std::map<WayPointSandbox, std::list<std::pair<StreetWayPoint, StreetWayPoint>>> way_point_edge_descriptors;
    {
        auto model_triangles = [&scene_node_resources](const std::string& resource_name) -> UUVector<FixedArray<ColoredVertex<float>, 3>>& {
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
        std::list<FixedArray<CompressedScenePos, 2, 2>> way_segments;
        ResourceNameCycle street_lights{ config.street_light_resource_names };
        RacingLineBvh racing_line_bvh;
        if (!config.racing_line_track.empty()) {
            load_racing_line_bvh(config.racing_line_track, normalization_matrix_, racing_line_bvh);
        }

        // draw_nodes(vertices, nodes, ways);
        // draw_test_lines(vertices, 0.02);
        // draw_ways(vertices, nodes, ways, 0.002);
        try {
            fg.update("Draw streets");
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
                config.included_aeroways,
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
                config.layer_heights,
                config.use_terrain_holes
            }};
        } catch (const PointException<CompressedScenePos, 3>& e) {
            handle_point_exception3(e, "Could not draw streets");
        } catch (const TriangleException<CompressedScenePos>& e) {
            handle_triangle_exception(e, "Could not draw streets");
        }
        try {
            project_nodes_onto_ways(mnodes, way_segments, config.scale);
        } catch (const PointException<CompressedScenePos, 2>& e) {
            handle_point_exception2(e, "Could not project nodes onto way");
        }
    }

    report_osm_problems(nodes, ways);

    std::list<Building> buildings;
    std::list<Building> wall_barriers;
    if (config.with_buildings || config.with_roofs || config.with_ceilings) {
        FacadeTextureCycle entrance_ftc{ config.entrance_textures };
        FacadeTextureCycle middle_ftc{ config.facade_textures };
        fg.update("Get buildings");
        buildings = get_buildings_or_wall_barriers(
            BuildingType::BUILDING,
            nodes,
            ways,
            config.scale,
            config.max_wall_width,
            config.building_bottom,
            config.default_building_top,
            config.default_snap_building_height,
            config.uv_scale_facade,
            config.socle_height,
            config.socle_textures,
            config.default_roof_9_2_max_building_height,
            config.default_roof_9_2.has_value()
                ? &*config.default_roof_9_2
                : nullptr,
            entrance_ftc,
            middle_ftc,
            config.default_building_vertical_subdivision);
    }
    {
        FacadeTextureCycle ftc({});
        fg.update("Get wall barriers");
        wall_barriers = get_buildings_or_wall_barriers(
            BuildingType::WALL_BARRIER,
            {},         // nodes
            ways,
            NAN,        // scale
            NAN,        // max_wall_width,
            config.building_bottom,
            config.default_barrier_top,
            config.default_snap_barrier_height,
            config.uv_scale_barrier_wall,
            NAN,        // socle_height
            {},
            INFINITY,   // default_roof_9_2_max_building_height
            nullptr,
            ftc,
            ftc,
            VerticalSubdivision::NONE);
    }

    fg.update("Determine terrain region contours");
    std::list<std::pair<TerrainType, std::list<FixedArray<CompressedScenePos, 2>>>> terrain_region_contours =
        get_terrain_region_contours(nodes, ways);
    WayBvh terrain_region_contours_bvh;
    for (const auto& [_, contour] : terrain_region_contours) {
        terrain_region_contours_bvh.add_path(contour);
    }

    GetMorphology get_building_morphology{
        Morphology{
            .physics_material = PhysicsMaterial::NONE,
            .center_distances = { 0.f, 300.f }
        },
        Morphology{
            .physics_material = PhysicsMaterial::NONE,
            .center_distances = { 300.f, INFINITY }
        },
        Morphology{
            .physics_material = PhysicsMaterial::NONE,
            .center_distances = { 0.f, INFINITY }
        }};

    if (config.with_buildings) {
        for (const auto& bu : buildings) {
            if (bu.way.nd.empty()) {
                lerr() << "Building " << bu.id << " is empty";
            } else if (bu.way.nd.front() != bu.way.nd.back()) {
                lerr() << "Building " << bu.id << " has no closed outline";
            }
        }
        fg.update("Draw building ground");
        try {
            draw_buildings_ceiling_or_ground(
                osm_triangle_lists.tls_buildings_ground,
                nullptr,
                Material{},
                get_building_morphology[BuildingDetailType::COMBINED],
                buildings,
                nodes,
                config.scale,
                config.triangulation_scale,
                config.uv_scale_ceiling,
                1.f,                     // uv_period
                config.max_wall_width,
                DrawBuildingPartType::GROUND,
                getenv_default("BUILDING_CONTOUR_TRIANGLES_FILENAME", ""),
                getenv_default("BUILDING_CONTOUR_FILENAME", ""),
                getenv_default("BUILDING_TRIANGLE_FILENAME", ""),
                config.contour_detection_strategy);
        } catch (const PointException<CompressedScenePos, 2>& e) {
            handle_point_exception2(e, "Could not triangulate building ground (BUILDING_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const p2t::PointException& e) {
            handle_point_exception(e, "Could not triangulate building ground (BUILDING_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const EdgeException<CompressedScenePos>& e) {
            handle_edge_exception(e, "Could not triangulate building ground (BUILDING_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const p2t::EdgeException& e) {
            handle_edge_exception(e, "Could not triangulate building ground (BUILDING_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const TriangleException<CompressedScenePos>& e) {
            handle_triangle_exception(e, "Could not triangulate building ground (BUILDING_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        }
    }

    auto all_hole_triangles = osm_triangle_lists.all_hole_triangles();
    auto street_hole_triangles = osm_triangle_lists.street_hole_triangles();
    auto building_hole_triangles = osm_triangle_lists.building_hole_triangles();
    auto ocean_ground_triangles = osm_triangle_lists.ocean_ground_triangles();
    StreetBvh all_holes_bvh{all_hole_triangles};
    StreetBvh ground_street_bvh{street_hole_triangles};
    StreetBvh air_bvh{air_triangle_lists.street_hole_triangles()};
    StreetBvh entrance_bvh{ osm_triangle_lists.entrance_triangles() };

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
        fg.update("Add beacons to raceways");
        add_beacons_to_raceways(
            scene_node_resources,
            *hri_.bri,
            nodes,
            ways,
            config.raceway_beacon_distance,
            config.scale);
    }
    {
        fg.update("Draw wall barriers");
        draw_wall_barriers(
            tls_wall_barriers,
            &steiner_points,
            vertex_height_bindings,
            Material{
                .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLOBS,
                .occluder_pass = ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC,
                .aggregate_mode = AggregateMode::ONCE,
                .shading = material_shading(PhysicsMaterial::SURFACE_BASE_STONE),
                .draw_distance_noperations = 1000},
            Morphology{ .physics_material = PhysicsMaterial::NONE },
            wall_barriers,
            nodes,
            config.scale,
            config.uv_scale_barrier_wall,
            config.max_wall_width,
            config.barrier_styles);
    }
    if (!config.boundary_barrier_style.empty()) {
        fg.update("Draw boundary barriers");
        try {
            draw_boundary_barriers(
                tls_wall_barriers,
                street_hole_triangles,
                Material{
                    .occluded_pass = ExternalRenderPassType::LIGHTMAP_BLOBS,
                    .occluder_pass = ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC,
                    .aggregate_mode = AggregateMode::ONCE,
                    .shading = material_shading(PhysicsMaterial::SURFACE_BASE_STONE),
                    .draw_distance_noperations = 1000},
                Morphology{ .physics_material = PhysicsMaterial::NONE },
                config.scale,
                config.uv_scale_barrier_wall,
                config.boundary_barrier_height,
                config.barrier_styles.get(config.boundary_barrier_style),
                config.contour_detection_strategy);
        } catch (const EdgeException<CompressedScenePos>& e) {
            handle_edge_exception(e, "Could not draw boundary barriers");
        }
    }

    std::vector<FixedArray<CompressedScenePos, 2>> map_outer_contour = get_map_outer_contour(
        nodes,
        ways);
    BoundingInfo bounding_info{ map_outer_contour, nodes, (CompressedScenePos)100.f, (CompressedScenePos)50.f };

    auto draw_terrain_triangles = [&config](TriangleList<CompressedScenePos>& dest, const std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>>& source){
        for (const auto& t : source) {
            auto uv = terrain_uv<CompressedScenePos, double>(
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
                Colors::from_rgb(terrain_color),
                Colors::from_rgb(terrain_color),
                Colors::from_rgb(terrain_color),
                uv[0],
                uv[1],
                uv[2]);
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
        //                     lerr() << std::setprecision(15) << v;
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
            std::vector<CompressedScenePos> coords = string_to_vector(str_getenv("MESH_AROUND_POS"), safe_stocs);
            if (coords.size() != 2) {
                THROW_OR_ABORT("MESH_AROUND_POS does not have length 2");
            }
            {
                FixedArray<CompressedScenePos, 3> pos{ coords[0], coords[1], (CompressedScenePos)0.f };
                auto m = get_geographic_mapping(TransformationMatrix<double, double, 3>::identity());
                lerr() << std::setprecision(15) << "Saving mesh around " << pos << " | " << m.transform(funpack(pos));
            }
            for (CompressedScenePos r : string_to_vector(str_getenv("MESH_AROUND_RADIUSES"), safe_stocs)) {
                plot_mesh(                         
                    FixedArray<size_t, 2>{2000u, 2000u},    // image_size
                    1,                                      // line_thickness
                    4,                                      // point_size
                    get_triangles_around(                   // triangles
                        all_hole_triangles,
                        {coords[0], coords[1]},
                        r),
                    {},                                     // contour
                    {{coords[0], coords[1], (CompressedScenePos)0.f}},  // highlighted_nodes
                    {}                                      // crossed_nodes
                    ).T().reversed(0).save_to_file(*prefix + "_r_" + std::to_string(r) + ".png");
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
        fg.update("Add street steiner points");
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
        fg.update("Triangulate terrain");
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
                config.triangulation_scale,
                config.uv_scale_terrain,
                config.uv_period_terrain,
                (CompressedScenePos)0.f,
                terrain_color,
                getenv_default("TERRAIN_CONTOUR_TRIANGLES_FILENAME", ""),
                getenv_default("TERRAIN_CONTOUR_FILENAME", ""),
                getenv_default("TERRAIN_TRIANGLE_FILENAME", ""),
                config.bounding_terrain_type,
                config.default_terrain_type,
                // Delete street holes because their untriangulated version is
                // used for terrain smoothing.
                { TerrainType::STREET_HOLE },
                config.contour_detection_strategy);
        } catch (const PointException<CompressedScenePos, 2>& e) {
            handle_point_exception2(e, "Could not triangulate terrain (TERRAIN_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const p2t::PointException& e) {
            handle_point_exception(e, "Could not triangulate terrain (TERRAIN_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const EdgeException<CompressedScenePos>& e) {
            handle_edge_exception(e, "Could not triangulate terrain (TERRAIN_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const p2t::EdgeException& e) {
            handle_edge_exception(e, "Could not triangulate terrain (TERRAIN_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const TriangleException<CompressedScenePos>& e) {
            handle_triangle_exception(e, "Could not triangulate terrain (TERRAIN_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        }
        for (const WaysideResourceNames& ws : config.waysides) {
            fg.update("Add grass on Steiner-points");
            ResourceNameCycle rnc{ ws.resource_names };
            add_grass_on_steiner_points(
                *hri_.bri,
                rnc,
                StreetBvh{ osm_triangle_lists.no_trees_triangles() },
                air_bvh,
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
    if (config.remove_backfacing_triangles) {
        fg.update("Remove backfacing triangles");
        auto prefix = try_getenv("BACKFACING_TRIANGLES_PREFIX");
        size_t i = 0;
        for (auto& l : std::list{&osm_triangle_lists, &air_triangle_lists}) {
            delete_backfacing_triangles(
                l->tls_no_backfaces(),
                !prefix.has_value()
                    ? ""
                    : *prefix + std::to_string(i) + ".png");
            ++i;
        }
    }

    std::list<FixedArray<CompressedScenePos, 3>> map_outer_contour3;
    for (const auto& p : map_outer_contour) {
        map_outer_contour3.emplace_back(p(0), p(1), (CompressedScenePos)0.f);
    }

    try {
        fg.update("Apply heightmap and smoothen");
        apply_heightmap_and_smoothen(
            config,
            ground_street_bvh,
            air_bvh,
            node_height_bindings,
            vertex_height_bindings,
            nodes,
            ways,
            normalized_points,
            tls_wall_barriers,
            osm_triangle_lists,
            air_triangle_lists,
            VertexOutOfHeightMapBehavior::THROW,
            *hri_.bri,
            steiner_points,
            map_outer_contour3,
            street_rectangles,
            way_point_edge_descriptors);
    } catch (const PointException<CompressedScenePos, 2>& e) {
        handle_point_exception2(e, "Could not apply heightmap and smoothen. Forgot to set map outer contour?");
    } catch (const TriangleException<CompressedScenePos>& e) {
        handle_triangle_exception(e, "Could not apply heightmap and smoothen.");
    }

    std::map<OrderableFixedArray<CompressedScenePos, 2>, FixedArray<CompressedScenePos, 3>> displacements;
    if (!config.displacementmap.empty()) {
        fg.update("Apply displacement map");
        apply_displacement_map(
            displacements,
            ground_street_bvh,
            air_bvh,
            osm_triangle_lists.tls_smoothed(),
            vertex_height_bindings,
            StbImage1::load_from_file(config.displacementmap).to_float_grayscale().casted<double>(),
            config.displacementmap_min,
            config.displacementmap_uv_scale,
            config.displacementmap_distance_2_z_scale,
            config.scale);
    } else {
        for (const auto& l : osm_triangle_lists.tls_smoothed()) {
            for (const auto& t : l->triangles) {
                for (const auto& v : t.flat_iterable()) {
                    auto v2 = OrderableFixedArray<CompressedScenePos, 2>(v.position(0), v.position(1));
                    displacements.try_emplace(v2, v.position);
                }
            }
        }
    }

    if (config.with_buildings) {
        fg.update("Draw building walls (facade)");
        draw_building_walls(
            tls_buildings,
            nullptr,            // Steiner points not required due to existence of ground triangles.
            displacements,
            Material{
                .occluder_pass = ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC,
                .aggregate_mode = AggregateMode::SORTED_CONTINUOUSLY,
                .shading = material_shading(PhysicsMaterial::SURFACE_BASE_STONE),
                .draw_distance_noperations = 1000},
            get_building_morphology[BuildingDetailType::COMBINED],
            buildings,
            nodes,
            config.scale,
            config.uv_scale_facade,
            config.max_wall_width,
            config.extrusion_ambient_occlusion,
            config.height_colors);
    }

    if (config.with_roofs) {
        fg.update("Draw roofs");
        auto& primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
        draw_roofs(
            tls_buildings,
            scene_node_resources,
            config.roof_model,
            displacements,
            Material{
                .textures_color = { primary_rendering_resources.get_blend_map_texture(config.roof_texture) },
                .occluder_pass = ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC,
                .aggregate_mode = AggregateMode::SORTED_CONTINUOUSLY,
                .shading = ROOF_REFLECTANCE,
                .draw_distance_noperations = 1000}.compute_color_mode(),
            Material{
                .textures_color = { primary_rendering_resources.get_blend_map_texture(config.roof_rail_texture) },
                .occluder_pass = ExternalRenderPassType::LIGHTMAP_BLACK_GLOBAL_STATIC,
                .aggregate_mode = AggregateMode::SORTED_CONTINUOUSLY,
                .shading = ROOF_REFLECTANCE,
                .draw_distance_noperations = 1000}.compute_color_mode(),
            get_building_morphology,
            roof_color,
            buildings,
            nodes,
            config.scale,
            config.uv_scale_roof,
            config.max_wall_width);
    }
    if (config.with_ceilings && !tls_buildings.empty()) {
        fg.update("Draw ceilings");
        try {
            draw_ceilings(
                tls_buildings,
                displacements,
                config,
                buildings,
                get_building_morphology[BuildingDetailType::COMBINED],
                nodes,
                getenv_default("CEILING_CONTOUR_TRIANGLES_FILENAME", ""),
                getenv_default("CEILING_CONTOUR_FILENAME", ""),
                getenv_default("CEILING_TRIANGLE_FILENAME", ""),
                config.contour_detection_strategy);
        } catch (const PointException<CompressedScenePos, 2>& e) {
            handle_point_exception2(e, "Could not triangulate ceilings (CEILING_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const p2t::PointException& e) {
            handle_point_exception(e, "Could not triangulate ceilings (CEILING_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const EdgeException<CompressedScenePos>& e) {
            handle_edge_exception(e, "Could not triangulate ceilings (CEILING_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const p2t::EdgeException& e) {
            handle_edge_exception(e, "Could not triangulate ceilings (CEILING_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const TriangleException<CompressedScenePos>& e) {
            handle_triangle_exception(e, "Could not triangulate ceilings (CEILING_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        }
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
    // If extrude_air_curb_amount is not zero,
    // boundaries have to be calculated at the ends of
    // air and ground street.
    try {
        fg.update("Extrude curbs, walls, grass, water");
        if (config.extrude_air_curb_amount == (CompressedScenePos)0.) {
            // If "extrude_air_curb_amount" IS NAN,
            // insert the air triangle lists here.
            osm_triangle_lists.insert(air_triangle_lists);
        } else if (config.extrude_air_curb_amount != (CompressedScenePos)0.) {
            TriangleList<CompressedScenePos>::extrude(
                *air_triangle_lists.tl_street_curb[RoadType::STREET],
                {
                    config.curb_alpha != config.curb2_alpha
                        ? air_triangle_lists.tl_street_curb[RoadType::STREET]
                        : air_triangle_lists.tl_street_curb2[RoadType::STREET]
                },
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
                TriangleList<CompressedScenePos>::extrude(
                    *air_triangle_lists.tl_street_curb[RoadType::PATH],
                    {
                        config.curb_alpha != config.curb2_alpha
                            ? air_triangle_lists.tl_street_curb[RoadType::PATH]
                            : air_triangle_lists.tl_street_curb2[RoadType::PATH]
                    },
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
        if (config.extrude_curb_amount != (CompressedScenePos)0.) {
            TriangleList<CompressedScenePos>::extrude(
                *osm_triangle_lists.tl_street_curb[RoadType::STREET],
                {
                    config.curb_alpha != config.curb2_alpha
                        ? osm_triangle_lists.tl_street_curb[RoadType::STREET]
                        : osm_triangle_lists.tl_street_curb2[RoadType::STREET]
                },
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
            if (osm_triangle_lists.tl_street_curb.contains(RoadType::PATH)) {
                TriangleList<CompressedScenePos>::extrude(
                    *osm_triangle_lists.tl_street_curb[RoadType::PATH],
                    {
                        config.curb_alpha != config.curb2_alpha
                            ? osm_triangle_lists.tl_street_curb[RoadType::PATH]
                            : osm_triangle_lists.tl_street_curb2[RoadType::PATH]
                    },
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
        if (config.extrude_wall_amount != (CompressedScenePos)0.) {
            TriangleList<CompressedScenePos>::extrude(
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
        if (config.extrude_air_curb_amount == (CompressedScenePos)0.) {
            raise_streets(
                osm_triangle_lists.tls_street_wo_curb(),
                osm_triangle_lists.tls_wo_subtraction_and_water(),
                config.scale,
                config.raise_streets_amount);
        } else {
            raise_streets(
                TriangleList<CompressedScenePos>::concatenated(
                    osm_triangle_lists.tls_street_wo_curb(),
                    air_triangle_lists.tls_street_wo_curb()),
                TriangleList<CompressedScenePos>::concatenated(
                    osm_triangle_lists.tls_wo_subtraction_and_water(),
                    air_triangle_lists.tls_wo_subtraction_and_water()),
                config.scale,
                config.raise_streets_amount);
        }
        if (config.extrude_grass_amount != (CompressedScenePos)0.) {
            TriangleList<CompressedScenePos>::extrude(
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
        if ((config.extrude_elevated_grass_amount != (CompressedScenePos)0.) &&
            osm_triangle_lists.tl_terrain->contains(TerrainType::ELEVATED_GRASS))
        {
            TriangleList<CompressedScenePos>::extrude(
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
        if ((config.extrude_water_floor_amout != (CompressedScenePos)0.) &&
            osm_triangle_lists.tl_terrain->contains(TerrainType::WATER_FLOOR))
        {
            std::set<OrderableFixedArray<CompressedScenePos, 3>> vertices_not_to_connect;
            for (const auto& p : map_outer_contour3) {
                vertices_not_to_connect.insert(OrderableFixedArray<CompressedScenePos, 3>{ p });
            }
            TriangleList<CompressedScenePos>::extrude(
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
    } catch (const EdgeException<CompressedScenePos>& e) {
        handle_edge_exception(e, "Extrusion failed");
    } catch (const TriangleException<CompressedScenePos>& e) {
        handle_triangle_exception(e, "Extrusion failed");
    }
    std::set<OrderableFixedArray<CompressedScenePos, 3>> boundary_vertices;
    // Compute boundary vertices.
    if ((config.extrude_street_amount != (CompressedScenePos)0.) ||
        (config.extrude_air_support_amount != (CompressedScenePos)0.))
    {
        fg.update("Compute vertices for street and air support extrusion");
        std::set<OrderableFixedArray<CompressedScenePos, 3>> terrain_vertices;
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
    if (config.extrude_street_amount != (CompressedScenePos)0.) {
        fg.update("Extrude streets");
        check_curb_validity(config.curb_alpha, config.curb2_alpha);
        if (!osm_triangle_lists.has_curb_or_curb2()) {  // "if (config.curb_alpha == 1)" not working for curbs from obj-models
            TriangleList<CompressedScenePos>::extrude(
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
                std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> source_triangles{triangle_lists.tls_curb_and_curb2()};
                TriangleList<CompressedScenePos>::extrude(
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
            if (config.extrude_air_curb_amount == (CompressedScenePos)0.) {
                do_extrude(osm_triangle_lists);
            } else {
                do_extrude(osm_triangle_lists);
                do_extrude(air_triangle_lists);
            }
        }
    }

    std::unique_ptr<GroundBvh> ground_bvh;
    {
        fg.update("Compute ground BVH");
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
                *ground_bvh,
                file_storage_type);
        }
        fg.update("Add models to model nodes");
        try {
            add_models_to_model_nodes(
                *hri_.bri,
                *ground_bvh,
                scene_node_resources,
                nodes,
                ways,
                config.game_level);
        } catch (const TriangleException<CompressedScenePos>& e) {
            if (auto prefix = try_getenv("EXCEPT_MESH_AROUND_PREFIX"); prefix.has_value()) {
                auto coords = (e.a + e.b + e.c) / 3;
                {
                    FixedArray<CompressedScenePos, 3> pos{ coords(0), coords(1), (CompressedScenePos)0.f };
                    auto m = get_geographic_mapping(TransformationMatrix<double, double, 3>::identity());
                    lerr() << std::setprecision(15) << "Saving mesh around " << pos << " | " << m.transform(funpack(pos));
                }
                for (CompressedScenePos r : string_to_vector(getenv_default("EXCEPT_MESH_AROUND_RADIUSES", "0.05 0.2 0.5"), safe_stocs)) {
                    plot_mesh(
                        FixedArray<size_t, 2>{ 2000u, 2000u },  // image_size
                        1,                                      // line_thickness
                        4,                                      // point_size
                        get_triangles_around(                   // triangles
                            ground_bvh_triangles,
                            { coords(0), coords(1) },
                            r),
                        {},                                     // contour
                        {                                       // highlighted_nodes
                            {e.a(0), e.a(1), (CompressedScenePos)0.f},
                            {e.b(0), e.b(1), (CompressedScenePos)0.f},
                            {e.c(0), e.c(1), (CompressedScenePos)0.f}
                        },
                        {}                                      // crossed_nodes
                    ).T().reversed(0).save_to_file(*prefix + "_r_" + std::to_string(r) + ".png");
                }
                handle_triangle_exception(e, "add models failed, debug image saved");
            } else {
                handle_triangle_exception(e, "add models failed, consider setting the 'EXCEPT_MESH_AROUND_PREFIX' environment variable");
            }
        }
    }

    if (config.with_tree_nodes && !config.tree_resource_names.empty()) {
        ResourceNameCycle rnc{ config.tree_resource_names };
        fg.update("Add trees to tree-nodes");
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

    if (!config.zonemap.empty()) {
        ResourceNameCycle rnc{ config.zone_resource_names };
        auto im = stb_load8(config.zonemap, FlipMode::VERTICAL);
        if (im.nrChannels != 1) {
            THROW_OR_ABORT("zonemap does not have 1 channel");
        }
        auto imf = gaussian_filter_NWE(
            stb_image_2_array(im)[0].casted<float>() / 255.f,
            1.f,                // sigma
            NAN,                // boundary
            4.f,                // truncate
            FilterExtension::PERIODIC);
        auto zonemap = 1.f - 2.f * abs(imf - 0.5f);
        fg.update("Add trees to zonemap");
        if (std::isnan(config.zonemap_width) || std::isnan(config.zonemap_height)) {
            THROW_OR_ABORT("zonemap width or height not set");
        }
        add_trees_to_zonemap(
            *hri_.bri,
            rnc,
            bounding_info,
            config.min_dist_to_road,
            all_holes_bvh,
            *ground_bvh,
            zonemap.casted<double>(),
            config.zonemap_width,
            config.zonemap_height,
            config.zonemap_multiplier,
            config.zonemap_jitter,
            config.zonemap_step_size,
            config.scale,
            config.water_height);
    }

    if (config.forest_outline_tree_distance != INFINITY && !config.tree_resource_names.empty()) {
        ResourceNameCycle rnc{config.tree_resource_names};
        fg.update("Add trees to forest outlines");
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
    }
    if (!config.road_bollard_resource_names.empty()) {
        ResourceNameCycle rnc{config.road_bollard_resource_names};
        fg.update("Draw road-bollards");
        draw_waysides(
            *hri_.bri,
            rnc,
            street_hole_triangles,
            *ground_bvh,
            entrance_bvh,
            config.scale,
            config.road_bollard_distances,
            config.contour_detection_strategy);
    }
    if (!config.trashcan_resource_names.empty()) {
        ResourceNameCycle rnc{config.trashcan_resource_names};
        fg.update("Draw trashcans");
        draw_waysides(
            *hri_.bri,
            rnc,
            street_hole_triangles,
            *ground_bvh,
            entrance_bvh,
            config.scale,
            config.trashcan_distances,
            config.contour_detection_strategy);
    }

    {
        // Extrude air support and raise tunnel crossings.
        const auto& air_or_osm = (config.extrude_air_curb_amount == (CompressedScenePos)0.)
            ? osm_triangle_lists
            : air_triangle_lists;

        fg.update("Flip air-support normals");
        // Must be after "delete_backfacing_triangles".
        air_or_osm.tl_air_support->flip();
        air_or_osm.tl_tunnel_crossing->flip();
        // Raise tunnel crossings.
        for (auto& t : air_or_osm.tl_tunnel_crossing->triangles) {
            for (auto& v : t.flat_iterable()) {
                v.position(2) += (CompressedScenePos)(config.default_tunnel_pipe_height * config.scale);
            }
        }

        // save_obj("/tmp/tl_terrain0.obj", IndexedFaceSet<float, size_t>{tl_terrain_->triangles_});
        std::set<const FixedArray<ColoredVertex<CompressedScenePos>, 3>*> triangles_to_delete;
        for (const auto& t : air_or_osm.tl_air_support->triangles) {
            if (boundary_vertices.contains(OrderableFixedArray{t(0).position}) ||
                boundary_vertices.contains(OrderableFixedArray{t(1).position}) ||
                boundary_vertices.contains(OrderableFixedArray{t(2).position}))
            {
                triangles_to_delete.insert(&t);
            }
        }
        if (config.extrude_air_support_amount != (CompressedScenePos)0.) {
            TriangleList<CompressedScenePos>::extrude(
                *air_or_osm.tl_air_support,                                 // dest
                {air_or_osm.tl_air_support},                                // triangle_lists
                nullptr,                                                    // follower_triangles
                nullptr,                                                    // source_triangles
                &boundary_vertices,                                         // clamped_vertices
                nullptr,                                                    // vertices_not_to_connect
                config.extrude_air_support_amount * config.scale,           // height
                config.scale,                                               // scale
                1,                                                          // uv_scale_x
                config.uv_scale_terrain,                                    // uv_scale_y
                false,                                                      // uvs_equal_lengths
                0.f);                                                       // ambient_occlusion
            air_or_osm.tl_air_support->triangles.remove_if([&triangles_to_delete](const FixedArray<ColoredVertex<CompressedScenePos>, 3>& t){
                return triangles_to_delete.contains(&t);
            });
        }
    }

    // for (auto& l : tls_ground) {
    //     colorize_height_map(l->triangles_);
    // }

    if (!config.grass_resource_names.empty() && (config.much_grass_distance != INFINITY)) {
        ResourceNameCycle rnc{ config.grass_resource_names };
        fg.update("Add grass inside triangles");
        add_grass_inside_triangles(
            *hri_.bri,
            rnc,
            *(*tl_terrain_)[TerrainType::GRASS],
            config.scale,
            CompressedScenePos::from_float_safe(config.much_grass_distance));
    }
    fg.update("Calculate spawn points");
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

    fg.update("Calculate normals");
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
        fg.update("Draw bumps");
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
    for (const auto& [visual_road_type, visual_e] : osm_triangle_lists.tl_street_mud_visuals.map()) {
        for (const auto& [physics_road_type, physics_e] : osm_triangle_lists.tl_street.list()) {
            if (physics_road_type.type != visual_road_type) {
                continue;
            }
            visual_e->triangles.insert(
                visual_e->triangles.end(),
                physics_e.triangle_list->triangles.begin(),
                physics_e.triangle_list->triangles.end());
        }
    }
    if (osm_triangle_lists.tl_street_mud_visuals.contains(RoadType::STREET)) {
        tl_mud_street_visuals_ = osm_triangle_lists.tl_street_mud_visuals[RoadType::STREET];
    }
    if (osm_triangle_lists.tl_street_mud_visuals.contains(RoadType::PATH)) {
        tl_mud_path_visuals_ = osm_triangle_lists.tl_street_mud_visuals[RoadType::PATH];
    }

    if (config.extrude_air_curb_amount != (CompressedScenePos)0.) {
        fg.update("Insert air triangles lists");
        // If "extrude_air_curb_amount" is NOT NAN,
        // insert the air triangle lists here.
        for (auto& l : air_triangle_lists.tl_street_curb.map()) {
            air_triangle_lists.tl_air_street_curb[l.first]->triangles = std::move(l.second->triangles);
        }
        osm_triangle_lists.insert(air_triangle_lists);
    }

    TriangleList<CompressedScenePos>::convert_triangle_to_vertex_normals(osm_triangle_lists.tls_with_vertex_normals());
    TriangleList<CompressedScenePos>::ambient_occlusion_by_curvature(osm_triangle_lists.tls_with_vertex_normals(), config.laplace_ambient_occlusion);
    TriangleList<CompressedScenePos>::convert_triangle_to_vertex_normals(tls_wall_barriers);

    std::list<std::shared_ptr<TriangleList<CompressedScenePos>>> tls_all;
    if (!config.water_texture->empty()) {
        std::list<std::pair<WaterType, std::list<FixedArray<CompressedScenePos, 2>>>> water_contours =
            get_water_region_contours(nodes, ways);
        fg.update("Triangulate water");
        try {
            triangulate_water(
                osm_triangle_lists.tl_water,
                bounding_info,
                {},                     // steiner_points
                map_outer_contour,
                {},                     // hole_triangles
                water_contours,
                config.scale,
                config.triangulation_scale,
                1 / 100.f,              // uv_scale
                1.f,                    // uv_period
                config.water_height,
                terrain_color,
                getenv_default("WATER_CONTOUR_TRIANGLES_FILENAME", ""),
                getenv_default("WATER_CONTOUR_FILENAME", ""),
                getenv_default("WATER_TRIANGLE_FILENAME", ""),
                WaterType::UNDEFINED,
                WaterType::UNDEFINED,
                config.contour_detection_strategy);
        } catch (const PointException<CompressedScenePos, 2>& e) {
            handle_point_exception2(e, "Could not triangulate water (WATER_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const p2t::PointException& e) {
            handle_point_exception(e, "Could not triangulate water (WATER_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const EdgeException<CompressedScenePos>& e) {
            handle_edge_exception(e, "Could not triangulate water (WATER_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const p2t::EdgeException& e) {
            handle_edge_exception(e, "Could not triangulate water (WATER_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        } catch (const TriangleException<CompressedScenePos>& e) {
            handle_triangle_exception(e, "Could not triangulate water (WATER_{CONTOUR_TRIANGLES|CONTOUR|TRIANGLE}_FILENAME environment variables for debugging)");
        }
        tls_all = osm_triangle_lists.tls_wo_subtraction_w_water();
    } else {
        tls_all = osm_triangle_lists.tls_wo_subtraction_and_water();
    }
    for (const auto& l : tls_buildings) {
        if (l->triangles.empty()) {
            THROW_OR_ABORT("Building \"" + l->name + "\" has no triangles");
        }
        buildings_.emplace_back(l->triangle_array());
    }
    for (const auto& l : std::list<const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>*>{
            &tls_all,
            &tls_wall_barriers })
    {
        for (const auto& l2 : *l) {
            if (!l2->triangles.empty()) {
                auto cva = l2->triangle_array();
                modulo_uv(*cva);
                hri_.acvas->dcvas.push_back(cva);
            }
        }
    }
    {
        fg.update("Calculate spawn-points");
        FacadeTextureCycle ftc({});
        std::list<Building> spawn_lines = get_buildings_or_wall_barriers(
            BuildingType::SPAWN_LINE,
            {},         // nodes
            ways,       // ways
            NAN,        // scale,
            NAN,        // max_wall_width,
            0,          // building_bottom
            0,          // default_building_top
            false,      // default_snap_height
            NAN,        // socle_height
            NAN,        // uv_scale_facade
            {},         // socle_textures
            INFINITY,   // default_roof_9_2_max_building_height
            nullptr,    // default_roof_9_2
            ftc,        // entrance_ftc
            ftc,        // middle_ftc
            VerticalSubdivision::NONE);
        try {
            for (const Building& bu : spawn_lines) {
                auto iteam = bu.way.tags.find("team");
                for (auto it = bu.way.nd.begin(); it != bu.way.nd.end(); ++it) {
                    auto next = it;
                    ++next;
                    if (next != bu.way.nd.end()) {
                        FixedArray<CompressedScenePos, 2> p = (nodes.at(*it).position + nodes.at(*next).position) / 2;
                        FixedArray<double, 2> dir = funpack(nodes.at(*it).position - nodes.at(*next).position);
                        double len2 = sum(squared(dir));
                        if (len2 < 1e-12) {
                            throw PointException{ p, "Spawn direction too small" };
                        }
                        dir /= std::sqrt(len2);
                        CompressedScenePos height;
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
        } catch (const PointException<CompressedScenePos, 2>& p) {
            handle_point_exception2(p, "Bould not apply height map to spawn lines");
        }
    }
    auto split_grass = [this, &ground_street_bvh, &fg](
        TerrainType source_terrain_type,
        TerrainType target_terrain_type,
        const TerrainStyleDistancesToBdry& target_terrain_distances_to_bdry)
    {
        if (target_terrain_distances_to_bdry.is_active) {
            if (auto tit = tl_terrain_->map().find(source_terrain_type); tit != tl_terrain_->map().end())
            {
                fg.update(
                    "Extract " + terrain_type_to_string(target_terrain_type) +
                    " from " + terrain_type_to_string(source_terrain_type));
                CompressedScenePos max_dist = (CompressedScenePos)(target_terrain_distances_to_bdry.max_distance_to_bdry * scale_);
                tl_terrain_->insert(target_terrain_type, std::make_shared<TriangleList<CompressedScenePos>>(
                    terrain_type_to_string(target_terrain_type) + "_autogen",
                    tit->second->material,
                    tit->second->morphology));
                auto& wayside_grass = *(*tl_terrain_)[target_terrain_type];
                tit->second->triangles.remove_if([&ground_street_bvh, &max_dist, &wayside_grass](const FixedArray<ColoredVertex<CompressedScenePos>, 3>& tri){
                    for (const auto& v : tri.flat_iterable()) {
                        if (ground_street_bvh.has_neighbor(
                            FixedArray<CompressedScenePos, 2>{
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
        fg.update("Add near hitboxes");
        cts.add_near_hitboxes(terrain_triangles(), street_bvh(), hri_);
        fg.update("Add far instances");
        cts.add_far_hitboxes(terrain_triangles(), street_bvh(), hri_);
    }
    fg.update("Save obj files if requested");
    save_to_obj_file_if_requested(debug_prefix);
    save_bad_triangles_to_obj_file_if_requested(debug_prefix);
    {
        fg.update("Calculate waypoints");
        std::list<TerrainWayPoints> terrain_way_point_lines = get_terrain_way_points(ways);
        try {
            if (config.with_street_way_points) {
                calculate_waypoint_adjacency(
                    way_points_[JoinedWayPointSandbox::STREET],
                    {},
                    WayPointsClass::NONE,
                    way_point_edge_descriptors[WayPointSandbox::STREET],
                    nodes,
                    *ground_bvh,
                    nullptr,        // to_meters
                    nullptr,        // sample_solo_mesh
                    config.scale,
                    config.waypoint_merge_radius,
                    config.waypoint_error_radius,
                    config.waypoint_distance);
            }
            if (config.with_sidewalk_way_points) {
                calculate_waypoint_adjacency(
                    way_points_[JoinedWayPointSandbox::SIDEWALK],
                    {},
                    WayPointsClass::NONE,
                    way_point_edge_descriptors[WayPointSandbox::SIDEWALK],
                    nodes,
                    *ground_bvh,
                    nullptr,        // to_meters
                    nullptr,        // sample_solo_mesh
                    config.scale,
                    config.waypoint_merge_radius,
                    config.waypoint_error_radius,
                    config.waypoint_distance);
            }
            if (!terrain_way_point_lines.empty() ||
                !way_point_edge_descriptors[WayPointSandbox::EXPLICIT_GROUND].empty())
            {
                const auto& navigation_dcvas = config.navmesh_resource.empty()
                    ? get_physics_arrays()->dcvas
                    : scene_node_resources.get_physics_arrays(config.navmesh_resource)->dcvas;
                if (!config.refine_explicit_waypoints) {
                    calculate_waypoint_adjacency(
                        way_points_[JoinedWayPointSandbox::EXPLICIT_GROUND],
                        terrain_way_point_lines,
                        WayPointsClass::GROUND,
                        way_point_edge_descriptors[WayPointSandbox::EXPLICIT_GROUND],
                        nodes,
                        *ground_bvh,
                        nullptr,        // to_meters
                        nullptr,        // sample_solo_mesh
                        config.scale,
                        config.waypoint_merge_radius,
                        config.waypoint_error_radius,
                        config.waypoint_distance);
                    calculate_waypoint_adjacency(
                        way_points_[JoinedWayPointSandbox::RUNWAY_OR_TAXIWAY_OR_AIRWAY],
                        terrain_way_point_lines,
                        WayPointsClass::AIRWAY,
                        way_point_edge_descriptors[WayPointSandbox::RUNWAY_OR_TAXIWAY],
                        nodes,
                        *ground_bvh,
                        nullptr,        // to_meters
                        nullptr,        // sample_solo_mesh
                        config.scale,
                        config.waypoint_merge_radius,
                        config.waypoint_error_radius,
                        config.waypoint_distance);
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
                    if (auto nm = try_getenv("OSM_NAVMESH_FILENAME"); nm.has_value()) {
                        save_obj(*nm, indexed_face_set, nullptr);
                    }
                    NavigationMeshBuilder nmb{
                        indexed_face_set,
                        NavigationMeshConfig{
                            .cell_size = 1.f,
                            .agent_radius = config.agent_radius}};
                    auto scaled_rotation = rotation.casted<double>() / scale_;
                    calculate_waypoint_adjacency(
                        way_points_[JoinedWayPointSandbox::EXPLICIT_GROUND],
                        terrain_way_point_lines,
                        WayPointsClass::GROUND,
                        way_point_edge_descriptors[WayPointSandbox::EXPLICIT_GROUND],
                        nodes,
                        *ground_bvh,
                        &scaled_rotation,
                        &nmb.ssm,
                        config.scale,
                        config.waypoint_merge_radius,
                        config.waypoint_error_radius,
                        config.waypoint_distance);
                    calculate_waypoint_adjacency(
                        way_points_[JoinedWayPointSandbox::RUNWAY_OR_TAXIWAY_OR_AIRWAY],
                        terrain_way_point_lines,
                        WayPointsClass::AIRWAY,
                        way_point_edge_descriptors[WayPointSandbox::RUNWAY_OR_TAXIWAY],
                        nodes,
                        *ground_bvh,
                        &scaled_rotation,
                        &nmb.ssm,
                        config.scale,
                        config.waypoint_merge_radius,
                        config.waypoint_error_radius,
                        config.waypoint_distance);
                }
            }
        } catch (const PointException<CompressedScenePos, 2>& e) {
            handle_point_exception2(e, "Could not calculate waypoint adjacency");
        } catch (const PointException<CompressedScenePos, 3>& e) {
            handle_point_exception3(e, "Could not calculate waypoint adjacency");
        } catch (const TriangleException<CompressedScenePos>& e) {
            handle_triangle_exception(e, "Could not calculate waypoint adjacency");
        } catch (const EdgeException<CompressedScenePos>& e) {
            handle_edge_exception(e, "Could not calculate waypoint adjacency");
        }
    }
    fg.update("Print waypoints if requested");
    print_waypoints_if_requested(debug_prefix);
}

OsmMapResource::OsmMapResource(
    SceneNodeResources& scene_node_resources,
    const std::string& level_filename,
    const std::string& debug_prefix)
    : hri_{ scene_node_resources, { NAN, NAN, NAN }, NAN }
    , scene_node_resources_{ scene_node_resources }
    , normalization_matrix_{ uninitialized }
    , triangulation_normalization_matrix_{ uninitialized }
    , terrain_styles_{}
{
    FunctionGuard fg{ "OSM map resource" };

    fg.update("Load OSM resource from cache");
    auto ifstr = create_ifstream(level_filename, std::ios::binary);
    if (ifstr->fail()) {
        THROW_OR_ABORT("Could not open input OSM-map binary file \"" + level_filename + '"');
    }
    cereal::BinaryInputArchive iarchive(*ifstr);
    iarchive(*this);
    if (ifstr->fail()) {
        THROW_OR_ABORT("Could not read from file \"" + level_filename + '"');
    }
    print_waypoints_if_requested(debug_prefix);
    save_to_obj_file_if_requested(debug_prefix);
    save_bad_triangles_to_obj_file_if_requested(debug_prefix);
}

const Bvh<CompressedScenePos, 3, FixedArray<CompressedScenePos, 3, 3>>& OsmMapResource::street_bvh() const {
    {
        std::shared_lock lock{street_bvh_mutex_};
        if (street_bvh_ != nullptr) {
            return *street_bvh_;
        }
    }
    if (street_bvh_ == nullptr) {
        std::scoped_lock lock{street_bvh_mutex_};
        street_bvh_.reset(new Bvh<CompressedScenePos, 3, FixedArray<CompressedScenePos, 3, 3>>{
            {(CompressedScenePos)0.1, (CompressedScenePos)0.1, (CompressedScenePos)0.1}, 10 });
        for (const auto& lst : tls_no_grass_) {
            for (const auto& t : lst->triangles) {
                FixedArray<CompressedScenePos, 3, 3> tri{
                    t(0).position,
                    t(1).position,
                    t(2).position};
                street_bvh_->insert(AxisAlignedBoundingBox<CompressedScenePos, 3>::from_points(tri), tri);
            }
        }
    }
    return *street_bvh_;
}

void OsmMapResource::save_to_file(
    const std::string& filename,
    FileStorageType file_storage_type) const
{
    auto ofstr = create_ofstream(filename, std::ios::binary, file_storage_type);
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
    const TransformationMatrix<float, double, 3>* tm) const
{
    auto filename = prefix + "_osm_map.obj";
    auto& primary_rendering_resources = RenderingContextStack::primary_rendering_resources();
    std::map<ColormapWithModifiers, std::string> autogen_textures;
    auto get_filename = [&](const ColormapWithModifiers& color, TextureRole role) -> std::string {
        if (color.filename->empty()) {
            return "";
        }
        auto it = autogen_textures.find(color);
        if (it == autogen_textures.end()) {
            std::string result = primary_rendering_resources.get_texture_filename(
                color,
                role,
                filename + ".tex." + std::to_string(autogen_textures.size()) + ".png");
            autogen_textures.insert({ color, result });
            return result;
        } else {
            return it->second;
        }
    };
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>> mdcvas;
    if (tm == nullptr) {
        mdcvas = hri_.acvas->dcvas;
    } else {
        for (const auto& l : hri_.acvas->dcvas) {
            mdcvas.push_back(l->transformed<CompressedScenePos>(*tm, ""));
        }
    }
    save_obj(
        filename,
        mdcvas,     // get_physics_arrays()->cvas
        {},         // material_name
        [&](const Material& m){
            ObjMaterial result{
                .ambient = m.shading.ambient,
                .diffuse = m.shading.diffuse,
                .specular = m.shading.specular};
            if (!m.textures_color.empty()) {
                const auto& desc = m.textures_color[0].texture_descriptor;
                result.color_texture = get_filename(desc.color, TextureRole::COLOR_FROM_DB);
                result.bump_texture = get_filename(desc.normal, TextureRole::NORMAL);
                result.has_alpha_texture = any(desc.color.color_mode & ColorMode::RGBA);
            }
            return result;
        });
}

void OsmMapResource::save_bad_triangles_to_obj_file(const std::string& filename) const {
    TriangleList<CompressedScenePos> bad_triangles{
        "bad_trinalges",
        Material{
            .shading{
                .ambient = {1.f, 0.f, 0.f},
                .diffuse = {1.f, 0.f, 0.f},
                .specular = {1.f, 0.f, 0.f}}},
        Morphology{ .physics_material = PhysicsMaterial::NONE } };
    for (const auto& l : hri_.acvas->dcvas) {
        for (const auto& t : l->triangles) {
            auto tlc = triangle_largest_cosine<ScenePos, 3>({
                funpack(t(0).position),
                funpack(t(1).position),
                funpack(t(2).position) });
            if (std::isnan(tlc) || (tlc > 0.999))
            {
                bad_triangles.triangles.push_back(t);
            }
        }
    }
    save_obj<CompressedScenePos>(filename, { bad_triangles.triangle_array() });
}

OsmMapResource::~OsmMapResource()
{}

void OsmMapResource::preload(const RenderableResourceFilter& filter) const {
    hri_.preload(filter);
    auto preload_styles = [&](const TerrainStyle& style) {
        if (style.is_visible()) {
            for (const auto& p : style.config.near_resource_names_valley_regular) {
                scene_node_resources_.preload_single(*p.name, filter);
            }
            for (const auto& p : style.config.near_resource_names_mountain_regular) {
                scene_node_resources_.preload_single(*p.name, filter);
            }
            for (const auto& p : style.config.near_resource_names_valley_dirt) {
                scene_node_resources_.preload_single(*p.name, filter);
            }
            for (const auto& p : style.config.near_resource_names_mountain_dirt) {
                scene_node_resources_.preload_single(*p.name, filter);
            }
        }
        };
    preload_styles(terrain_styles_.street_mud_terrain_style);
    preload_styles(terrain_styles_.path_mud_terrain_style);
    preload_styles(terrain_styles_.near_grass_terrain_style);
    preload_styles(terrain_styles_.far_grass_terrain_style);
    preload_styles(terrain_styles_.near_wayside1_grass_terrain_style);
    preload_styles(terrain_styles_.near_wayside2_grass_terrain_style);
    preload_styles(terrain_styles_.near_flowers_terrain_style);
    preload_styles(terrain_styles_.far_flowers_terrain_style);
    preload_styles(terrain_styles_.near_trees_terrain_style);
    preload_styles(terrain_styles_.far_trees_terrain_style);
    preload_styles(terrain_styles_.no_grass_decals_terrain_style);
}

TerrainTriangles OsmMapResource::terrain_triangles() const {
    return TerrainTriangles{
        .grass = tl_terrain_->contains(TerrainType::GRASS) ? &(*tl_terrain_)[TerrainType::GRASS]->triangles : nullptr,
        .elevated_grass = tl_terrain_->contains(TerrainType::ELEVATED_GRASS) ? &(*tl_terrain_)[TerrainType::ELEVATED_GRASS]->triangles : nullptr,
        .wayside1_grass = tl_terrain_->contains(TerrainType::WAYSIDE1_GRASS) ? &(*tl_terrain_)[TerrainType::WAYSIDE1_GRASS]->triangles : nullptr,
        .wayside2_grass = tl_terrain_->contains(TerrainType::WAYSIDE2_GRASS) ? &(*tl_terrain_)[TerrainType::WAYSIDE2_GRASS]->triangles : nullptr,
        .flowers = tl_terrain_->contains(TerrainType::FLOWERS) ? &(*tl_terrain_)[TerrainType::FLOWERS]->triangles : nullptr,
        .trees = tl_terrain_->contains(TerrainType::TREES) ? &(*tl_terrain_)[TerrainType::TREES]->triangles : nullptr,
        .street_mud_grass = tl_mud_street_visuals_ != nullptr ? &tl_mud_street_visuals_->triangles : nullptr,
        .path_mud_grass = tl_mud_path_visuals_ != nullptr ? &tl_mud_path_visuals_->triangles : nullptr,
    };
}

std::list<const UUList<FixedArray<ColoredVertex<CompressedScenePos>, 3>>*> OsmMapResource::no_grass() const {
    std::list<const UUList<FixedArray<ColoredVertex<CompressedScenePos>, 3>>*> result;
    for (const auto& lst : tls_no_grass_) {
        result.push_back(&lst->triangles);
    }
    return result;
}

void OsmMapResource::instantiate_root_renderables(const RootInstantiationOptions& options) const
{
    hri_.instantiate_root_renderables(options);
    for (const auto& [i, b] : enumerate(buildings_)) {
        auto center = b->aabb().data().center();
        auto tm = TranslationMatrix{ center.casted<ScenePos>() };
        auto trafo = options.absolute_model_matrix * tm;
        auto rcva = std::make_shared<ColoredVertexArrayResource>(b->translated<CompressedScenePos>(-center, "_centered"));
        rcva->instantiate_root_renderables(
            RootInstantiationOptions{
                .rendering_resources = options.rendering_resources,
                .instance_name = VariableAndHash<std::string>{ "building_" + std::to_string(i) },
                .absolute_model_matrix = trafo,
                .scene = options.scene,
                .renderable_resource_filter = options.renderable_resource_filter
            });
    }
    if (terrain_styles_.requires_renderer()) {
        auto node = make_unique_scene_node(
            options.absolute_model_matrix.t,
            matrix_2_tait_bryan_angles(options.absolute_model_matrix.R),
            options.absolute_model_matrix.get_scale(),
            PoseInterpolationMode::DISABLED);
        node->add_renderable(VariableAndHash<std::string>{ "osm_map_near" }, std::make_shared<RenderableTriangleSampler>(
            scene_node_resources_,
            terrain_styles_,
            terrain_triangles(),
            no_grass(),
            &street_bvh(),
            scale_,
            UpAxis::Z));
        options.scene.auto_add_root_node(
            *options.instance_name + "_osm_near_world",
            std::move(node),
            RenderingDynamics::STATIC);
    }
    // if (rbvh_ == nullptr) {
    //     rbvh_ = std::make_shared<BvhResource>(cvas_);
    // }
    // rbvh_->instantiate_child_renderable(options);
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
    } catch (const TriangleException<CompressedScenePos>& e) {
        handle_triangle_exception(e, "Could not decompose terrain into convex regions");
    }
}

void OsmMapResource::smoothen_edges(
    SmoothnessTarget target,
    float smoothness,
    size_t niterations,
    float decay)
{
    hri_.smoothen_edges(target, smoothness, niterations, decay);
}

TransformationMatrix<double, double, 3> OsmMapResource::get_geographic_mapping(
    const TransformationMatrix<double, double, 3>& absolute_model_matrix) const
{
    return get_geographic_mapping_3d(normalization_matrix_, absolute_model_matrix, scale_);
}

std::list<SpawnPoint> OsmMapResource::get_spawn_points() const {
    return spawn_points_;
}

WayPointSandboxes OsmMapResource::get_way_points() const {
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

static void plot_way_points_and_obstacles(
    const std::string& filename,
    const PointsAndAdjacencyResource& pa,
    const std::list<FixedArray<CompressedScenePos, 2>>& bounding_contour,
    const std::list<FixedArray<CompressedScenePos, 3>>& hitbox_positions)
{
    std::list<FixedArray<CompressedScenePos, 3, 2>> triangles;
    std::list<FixedArray<CompressedScenePos, 2, 2>> edges;
    std::list<std::list<FixedArray<CompressedScenePos, 2>>> contours;
    std::list<FixedArray<CompressedScenePos, 2>> highlighted_nodes;
    for (size_t c = 0; c < pa.adjacency.shape(1); ++c) {
        for (const auto& [r, _] : pa.adjacency.column(c)) {
            if (r != c) {
                edges.push_back(FixedArray<CompressedScenePos, 2, 2>{
                    FixedArray<CompressedScenePos, 2>{pa.points.at(c).position(0), pa.points.at(c).position(1)},
                    FixedArray<CompressedScenePos, 2>{pa.points.at(r).position(0), pa.points.at(r).position(1)}});
            }
        }
    }
    contours.push_back(bounding_contour);
    for (const auto& p : hitbox_positions) {
        highlighted_nodes.push_back(FixedArray<CompressedScenePos, 2>{p(0), p(1)});
    }
    if (!edges.empty() || !highlighted_nodes.empty()) {
        plot_mesh_svg(filename, 600., 600., triangles, edges, contours, highlighted_nodes);
    }
}

void OsmMapResource::print_waypoints_if_requested(const std::string& debug_prefix) const {
    if (auto wf = try_getenv("OSM_WAYPOINT_PREFIX"); wf.has_value()) {
        auto rs = try_getenv("OSM_WAYPOINT_BBOX_RADIUS");
        if (!rs.has_value()) {
            THROW_OR_ABORT("Please specify the \"OSM_WAYPOINT_BBOX_RADIUS\" environment variable (should be in the range 1 - 2)");
        }
        CompressedScenePos r = safe_stocs(*rs);
        // way_points_.at(WayPointLocation::STREET).plot(wf + debug_prefix + "street.svg", 600, 600, 0.1f);
        // way_points_.at(WayPointLocation::SIDEWALK).plot(wf + debug_prefix + "sidewalk.svg", 600, 600, 0.1f);
        // way_points_.at(WayPointLocation::EXPLICIT).plot(wf + debug_prefix + "explicit.svg", 600, 600, 0.1f);

        std::list<FixedArray<CompressedScenePos, 2>> bounding_contour{
            FixedArray<CompressedScenePos, 2>{-r, -r},
            FixedArray<CompressedScenePos, 2>{+r, -r},
            FixedArray<CompressedScenePos, 2>{+r, +r},
            FixedArray<CompressedScenePos, 2>{-r, +r},
            FixedArray<CompressedScenePos, 2>{-r, -r}};
        auto hitbox_positions = hri_.bri->hitbox_positions();
        if (auto it = way_points_.find(JoinedWayPointSandbox::STREET); it != way_points_.end()) {
            plot_way_points_and_obstacles(*wf + debug_prefix + "street.svg", it->second, bounding_contour, hitbox_positions);
        }
        if (auto it = way_points_.find(JoinedWayPointSandbox::SIDEWALK); it != way_points_.end()) {
            plot_way_points_and_obstacles(*wf + debug_prefix + "sidewalk.svg", it->second, bounding_contour, hitbox_positions);
        }
        if (auto it = way_points_.find(JoinedWayPointSandbox::EXPLICIT_GROUND); it != way_points_.end()) {
            plot_way_points_and_obstacles(*wf + debug_prefix + "explicit_ground.svg", it->second, bounding_contour, hitbox_positions);
        }
        if (auto it = way_points_.find(JoinedWayPointSandbox::RUNWAY_OR_TAXIWAY_OR_AIRWAY); it != way_points_.end()) {
            plot_way_points_and_obstacles(*wf + debug_prefix + "runway_or_taxiway_or_airway.svg", it->second, bounding_contour, hitbox_positions);
        }
    }
}

void OsmMapResource::save_to_obj_file_if_requested(const std::string& debug_prefix) const
{
    if (auto wp = try_getenv("OSM_OBJ_PREFIX"); wp.has_value()) {
        save_to_obj_file(*wp + debug_prefix + ".obj", nullptr);
    }
}

void OsmMapResource::save_bad_triangles_to_obj_file_if_requested(const std::string& debug_prefix) const {
    if (auto wp = try_getenv("OSM_BAD_OBJ_PREFIX"); wp.has_value()) {
        save_bad_triangles_to_obj_file(wp.value() + debug_prefix + ".obj");
    }
}

void OsmMapResource::handle_point_exception3(
    const PointException<CompressedScenePos, 3>& e,
    const std::string& message) const
{
    auto m = get_geographic_mapping(TransformationMatrix<double, double, 3>::identity());
    throw std::runtime_error(e.str(message, &m));
}

void OsmMapResource::handle_point_exception2(
    const PointException<CompressedScenePos, 2>& e,
    const std::string& message) const
{
    using C = CompressedScenePos;
    handle_point_exception3(PointException<C, 3>{FixedArray<C, 3>{ e.point(0), e.point(1), (C)0. }, e.what()}, message);
}

void OsmMapResource::handle_point_exception(
    const p2t::PointException& e,
    const std::string& message) const
{
    FixedArray<double, 3> pos{e.point.x, e.point.y, 0.};
    auto m = get_geographic_mapping(TransformationMatrix<double, double, 3>::identity());
    std::stringstream sstr;
    sstr.precision(15);
    sstr << message << " at position " << m.transform(pos) << " | " << pos << ": " << e.what() << std::endl;
    throw std::runtime_error(sstr.str());
}

void OsmMapResource::handle_edge_exception(
    const p2t::EdgeException& e,
    const std::string& message) const
{
    FixedArray<double, 2, 3> edge{
        FixedArray<double, 3>{ e.edge[0].x, e.edge[0].y, 0.},
        FixedArray<double, 3>{ e.edge[1].x, e.edge[1].y, 0.} };
    auto m = get_geographic_mapping(TransformationMatrix<double, double, 3>::identity());
    std::stringstream sstr;
    sstr.precision(15);
    sstr << message << " at edge " <<
        m.transform(edge[0]) << " | " << edge[0] << " <-> " <<
        m.transform(edge[1]) << " | " << edge[1] <<
        ": " << e.what() << std::endl;
    throw std::runtime_error(sstr.str());
}

void OsmMapResource::handle_edge_exception(
    const EdgeException<CompressedScenePos>& e,
    const std::string& message) const
{
    auto m = get_geographic_mapping(TransformationMatrix<double, double, 3>::identity());
    throw std::runtime_error(e.str(message, &m));
}

void OsmMapResource::handle_triangle_exception(
    const TriangleException<CompressedScenePos>& e,
    const std::string& message) const
{
    auto m = get_geographic_mapping(TransformationMatrix<double, double, 3>::identity());
    throw std::runtime_error(e.str(message, &m));
}
