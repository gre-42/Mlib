#include "Renderable_Osm_Map.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Normalized_Points_Fixed.hpp>
#include <Mlib/Images/PgmImage.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Geographic_Coordinates.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Render/Renderables/Renderable_Osm_Map_Helpers.hpp>
#include <Mlib/Render/Rendering_Resources.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/String.hpp>
#include <regex>

#undef LOG_FUNCTION
#undef LOG_INFO
#define LOG_FUNCTION(msg) ::Mlib::Log log(msg, "LOG_OSM_MAP")
#define LOG_INFO(msg) log.info(msg)

using namespace Mlib;

RenderableOsmMap::RenderableOsmMap(
    SceneNodeResources& scene_node_resources,
    RenderingResources* rendering_resources,
    const std::string& filename,
    const std::string& heightmap,
    const std::string& terrain_texture,
    const std::string& dirt_texture,
    const std::string& asphalt_texture,
    const std::string& street_texture,
    const std::string& path_texture,
    const std::string& curb_street_texture,
    const std::string& curb_path_texture,
    const std::string& facade_texture,
    const std::string& facade_texture_2,
    const std::string& facade_texture_3,
    const std::string& ceiling_texture,
    const std::string& barrier_texture,
    BlendMode barrier_blend_mode,
    const std::string& roof_texture,
    const std::vector<std::string>& tree_resource_names,
    const std::vector<std::string>& grass_resource_names,
    const std::list<WaysideResourceNames>& waysides,
    float default_street_width,
    float roof_width,
    float scale,
    float uv_scale_terrain,
    float uv_scale_street,
    float uv_scale_facade,
    float uv_scale_ceiling,
    float uv_scale_barrier_wall,
    bool with_roofs,
    bool with_ceilings,
    float building_bottom,
    float default_building_top,
    float default_barrier_top,
    bool remove_backfacing_triangles,
    bool with_tree_nodes,
    float forest_outline_tree_distance,
    float forest_outline_tree_inwards_distance,
    float much_grass_distance,
    float raceway_beacon_distance,
    bool with_terrain,
    bool with_buildings,
    bool only_raceways,
    const std::string& highway_name_pattern,
    const std::set<std::string>& path_tags,
    const std::vector<float>& steiner_point_distances_road,
    const std::vector<float>& steiner_point_distances_steiner,
    float curb_alpha,
    float raise_streets_amount,
    float extrude_curb_amount,
    bool add_street_lights,
    float max_wall_width,
    bool with_height_bindings,
    float street_node_smoothness,
    float street_edge_smoothness,
    float terrain_edge_smoothness)
: rendering_resources_{rendering_resources},
  scene_node_resources_{scene_node_resources},
  scale_{scale}
{
    LOG_FUNCTION("RenderableOsmMap::RenderableOsmMap");
    std::ifstream ifs{filename};
    const std::regex node_reg("^ +<node id=[\"'](-?\\w+)[\"'] .*visible=[\"'](true|false)[\"'].* lat=[\"']([\\w.-]+)[\"'] lon=[\"']([\\w.-]+)[\"'].*>$");
    const std::regex way_reg("^ +<way id=[\"'](-?\\w+)[\"'].* visible=[\"'](true|false)[\"'].*>$");
    const std::regex way_end_reg("^ +</way>$");
    const std::regex node_ref_reg("^  +<nd ref=[\"'](-?\\w+)[\"'] */>$");
    const std::regex bounds_reg(" +<bounds minlat=[\"']([\\w.-]+)[\"'] minlon=[\"']([\\w.-]+)[\"'] maxlat=[\"']([\\w.-]+)[\"'] maxlon=[\"']([\\w.-]+)[\"'](?: origin=[\"'].*[\"'])? */>$");
    const std::regex tag_reg("  +<tag k=[\"']([\\w:;.&|*=-]+)[\"'] v=[\"'](.*)[\"'] */>$");
    const std::regex ignored_reg(
        "^(?:"
        "<\\?xml .*|"
        "<osm .*|"
        " +</node>|"
        " +<relation .*|"
        "  +<member .*|"
        " +</relation>|"
        "</osm>)$");

    NormalizedPointsFixed normalized_points{ScaleMode::NONE, OffsetMode::CENTERED};

    FixedArray<double, 2> coords_ref;
    FixedArray<float, 2, 3> normalization_matrix;
    bool normalization_matrix_defined = false;
    std::string current_way = "<none>";
    std::string current_node = "<none>";
    std::map<std::string, Node> nodes;
    std::map<OrderableFixedArray<float, 2>, std::string> ordered_node_positions;
    std::map<std::string, Way> ways;

    std::string line;
    while(std::getline(ifs, line)) {
        if (line.length() > 0 && line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1);
        }
        if (line.length() == 0) {
            continue;
        }
        std::smatch match;
        if (std::regex_match(line, ignored_reg)) {
            // do nothing
        } else if (std::regex_match(line, match, bounds_reg)) {
            FixedArray<double, 2> bounds_min{
                safe_stod(match[1].str()),
                safe_stod(match[2].str())};
            FixedArray<double, 2> bounds_max{
                safe_stod(match[3].str()),
                safe_stod(match[4].str())};
            coords_ref = (bounds_max + bounds_min) / 2.0;
            FixedArray<float, 2> min;
            FixedArray<float, 2> max;
            latitude_longitude_2_meters(
                safe_stod(match[1].str()),
                safe_stod(match[2].str()),
                coords_ref(0),
                coords_ref(1),
                min(0),
                min(1));
            latitude_longitude_2_meters(
                safe_stod(match[3].str()),
                safe_stod(match[4].str()),
                coords_ref(0),
                coords_ref(1),
                max(0),
                max(1));
            normalized_points.add_point(min * scale);
            normalized_points.add_point(max * scale);
            normalization_matrix = normalized_points.normalization_matrix();
            normalization_matrix_defined = true;
        } else if (std::regex_match(line, match, node_reg)) {
            current_way = "<none>";
            current_node = match[1].str();
            if (!normalization_matrix_defined) {
                throw std::runtime_error("Normalization-matrix undefined, bounds-section?");
            }
            if (match[2].str() == "true") {
                FixedArray<float, 2> geo;
                latitude_longitude_2_meters(
                    safe_stod(match[3].str()),
                    safe_stod(match[4].str()),
                    coords_ref(0),
                    coords_ref(1),
                    geo(0),
                    geo(1));
                geo *= scale;
                if (nodes.find(match[1].str()) != nodes.end()) {
                    throw std::runtime_error("Found duplicate node id: " + match[1].str());
                }
                auto pos = dot1d(normalization_matrix, homogenized_3(geo));
                auto opos = OrderableFixedArray<float, 2>{pos};
                auto it = ordered_node_positions.find(opos);
                if (it != ordered_node_positions.end()) {
                    std::cerr << "Detected duplicate points: " + match[1].str() + ", " + it->second << std::endl;
                } else {
                    ordered_node_positions.insert(std::make_pair(opos, match[1].str()));
                }
                nodes.insert(std::make_pair(match[1].str(), Node{position: pos}));
                // float dist = sum(squared(pos - FixedArray<float, 2>{-0.801262, 0.0782831}));
                // if (dist < 1e-3) {
                //     std::cerr << "err: " << dist << " " << match[1].str() << std::endl;
                // }
            }
        } else if (std::regex_match(line, match, way_reg)) {
            current_node = "<none>";
            if (match[2].str() == "true") {
                current_way = match[1].str();
                ways.insert(std::make_pair(current_way, Way{}));
            } else {
                current_way = "<invisible>";
            }
        } else if (std::regex_match(line, match, node_ref_reg)) {
            if (current_way == "<none>") {
                throw std::runtime_error("No current way");
            }
            if (current_way != "<invisible>") {
                ways.at(current_way).nd.push_back(match[1].str());
            }
        } else  if (std::regex_match(line, match, tag_reg)) {
            assert_true((current_node == "<none>") || (current_way == "<none>"));
            auto tag = std::make_pair(match[1].str(), match[2].str());
            if (current_node != "<none>") {
                if (!nodes.at(current_node).tags.insert(tag).second) {
                    throw std::runtime_error("Duplicate node tag " + tag.first);
                }
            }
            if (current_way != "<none>") {
                if (current_way != "<invisible>") {
                    if (!ways.at(current_way).tags.insert(tag).second) {
                        throw std::runtime_error("Duplicate way tag " + tag.first);
                    }
                }
            }
        } else if (std::regex_match(line, way_end_reg)) {
            current_way = "<none>";
        } else {
            throw std::runtime_error("Could not parse line " + line);
        }
    }

    if (!ifs.eof() && ifs.fail()) {
        throw std::runtime_error("Error reading from file " + filename);
    }

    std::list<Building> buildings;
    std::list<Building> wall_barriers;
    if (with_buildings || with_roofs || with_ceilings) {
        buildings = get_buildings_or_wall_barriers(
            BuildingType::BUILDING,
            ways,
            building_bottom,
            default_building_top);
        compute_building_area(
            buildings,
            nodes,
            scale);
    }
    {
        wall_barriers = get_buildings_or_wall_barriers(
            BuildingType::WALL_BARRIER,
            ways,
            building_bottom,
            default_barrier_top);
    }

    auto tl_terrain = std::make_shared<TriangleList>("terrain", Material{
        .texture_descriptor = {.color = terrain_texture, .normal = rendering_resources->get_normalmap(terrain_texture)},
        .dirt_texture = dirt_texture,
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .specularity = {0.2, 0.2, 0.2}}.compute_color_mode());
    auto tl_street_crossing = std::make_shared<TriangleList>("street_crossing", Material{
        .texture_descriptor = {.color = asphalt_texture, .normal = rendering_resources->get_normalmap(asphalt_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .specularity = {0.2, 0.2, 0.2}}.compute_color_mode());
    auto tl_path_crossing = std::make_shared<TriangleList>("path_crossing", Material{
        .texture_descriptor = {.color = path_texture, .normal = rendering_resources->get_normalmap(path_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .specularity = {0.2, 0.2, 0.2}}.compute_color_mode());
    auto tl_street = std::make_shared<TriangleList>("street", Material{
        .texture_descriptor = {.color = street_texture, .normal = rendering_resources->get_normalmap(street_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .specularity = {0.2, 0.2, 0.2}}.compute_color_mode()); // mixed_texture: terrain_texture
    auto tl_path = std::make_shared<TriangleList>("path", Material{
        .texture_descriptor = {.color = path_texture, .normal = rendering_resources->get_normalmap(path_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .specularity = {0.2, 0.2, 0.2}}.compute_color_mode()); // mixed_texture: terrain_texture
    auto tl_curb_street = std::make_shared<TriangleList>("curb_street", Material{
        .texture_descriptor = {.color = curb_street_texture, .normal = rendering_resources->get_normalmap(curb_street_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .wrap_mode_s = extrude_curb_amount != 0 ? WrapMode::REPEAT : WrapMode::CLAMP_TO_EDGE,
        .specularity = {0.2, 0.2, 0.2}}.compute_color_mode()); // mixed_texture: terrain_texture
    auto tl_curb_path = std::make_shared<TriangleList>("curb_path", Material{
        .texture_descriptor = {.color = curb_path_texture, .normal = rendering_resources->get_normalmap(curb_path_texture)},
        .occluded_type = OccludedType::LIGHT_MAP_COLOR,
        .occluder_type = OccluderType::WHITE,
        .wrap_mode_s = extrude_curb_amount != 0 ? WrapMode::REPEAT : WrapMode::CLAMP_TO_EDGE,
        .specularity = {0.2, 0.2, 0.2}}.compute_color_mode()); // mixed_texture: terrain_texture
    std::list<std::shared_ptr<TriangleList>> tls_ground{tl_terrain, tl_street_crossing, tl_path_crossing, tl_street, tl_path, tl_curb_street, tl_curb_path};
    std::list<std::shared_ptr<TriangleList>> tls_ground_wo_curb{tl_terrain, tl_street_crossing, tl_path_crossing, tl_street, tl_path};
    std::list<std::shared_ptr<TriangleList>> tls_buildings;
    std::list<std::shared_ptr<TriangleList>> tls_wall_barriers;
    std::map<OrderableFixedArray<float, 2>, std::set<std::string>> height_bindings;
    std::list<SteinerPointInfo> steiner_points;
    {
        std::vector<FixedArray<float, 2>> map_outer_contour;
        get_map_outer_contour(
            map_outer_contour,
            nodes,
            ways);

        // draw_nodes(vertices, nodes, ways);
        // draw_test_lines(vertices, 0.02);
        // draw_ways(vertices, nodes, ways, 0.002);
        draw_streets(
            *tl_street_crossing,
            *tl_path_crossing,
            *tl_street,
            *tl_path,
            *tl_curb_street,
            *tl_curb_path,
            object_resource_descriptors_,
            height_bindings,
            nodes,
            ways,
            scale,
            uv_scale_street,
            default_street_width,
            only_raceways,
            highway_name_pattern,
            path_tags,
            curb_alpha,
            add_street_lights,
            with_height_bindings);

        if (forest_outline_tree_distance != INFINITY && !tree_resource_names.empty()) {
            ResourceNameCycle rnc{scene_node_resources, tree_resource_names};
            LOG_INFO("add_trees_to_forest_outlines");
            add_trees_to_forest_outlines(
                resource_instance_positions_,
                object_resource_descriptors_,
                steiner_points,
                rnc,
                nodes,
                ways,
                forest_outline_tree_distance,
                forest_outline_tree_inwards_distance,
                scale);
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
        if (raceway_beacon_distance != INFINITY) {
            LOG_INFO("add_beacons_to_raceways");
            add_beacons_to_raceways(
                object_resource_descriptors_,
                nodes,
                ways,
                raceway_beacon_distance,
                scale);
        }
        if (with_tree_nodes && !tree_resource_names.empty()) {
            ResourceNameCycle rnc{scene_node_resources, tree_resource_names};
            LOG_INFO("add_trees_to_tree_nodes");
            add_trees_to_tree_nodes(
                resource_instance_positions_,
                object_resource_descriptors_,
                steiner_points,
                rnc,
                nodes,
                scale);
        }

        if (with_buildings) {
            LOG_INFO("draw_building_walls (facade)");
            draw_building_walls(
                tls_buildings,
                steiner_points,
                Material{
                    .texture_descriptor = {color: "<tbd>"},
                    .aggregate_mode = AggregateMode::ONCE,
                    .ambience = {1, 1, 1}}.compute_color_mode(),
                buildings,
                nodes,
                scale,
                uv_scale_facade,
                max_wall_width,
                facade_texture,
                facade_texture_2,
                facade_texture_3);
        }
        {
            LOG_INFO("draw_building_walls (barrier)");
            draw_building_walls(
                tls_wall_barriers,
                steiner_points,
                Material{
                    .texture_descriptor = {color: "<tbd>"},
                    .occluder_type = OccluderType::OFF,
                    .blend_mode = barrier_blend_mode,
                    .aggregate_mode = AggregateMode::ONCE,
                    .is_small = false,
                    .cull_faces = false}.compute_color_mode(),
                wall_barriers,
                nodes,
                scale,
                uv_scale_barrier_wall,
                max_wall_width,
                barrier_texture,
                barrier_texture,
                barrier_texture);
        }

        if (with_terrain) {
            auto hole_triangles = tl_street_crossing->triangles_;
            hole_triangles.insert(hole_triangles.end(), tl_path_crossing->triangles_.begin(), tl_path_crossing->triangles_.end());
            hole_triangles.insert(hole_triangles.end(), tl_street->triangles_.begin(), tl_street->triangles_.end());
            hole_triangles.insert(hole_triangles.end(), tl_path->triangles_.begin(), tl_path->triangles_.end());
            hole_triangles.insert(hole_triangles.end(), tl_curb_street->triangles_.begin(), tl_curb_street->triangles_.end());
            hole_triangles.insert(hole_triangles.end(), tl_curb_path->triangles_.begin(), tl_curb_path->triangles_.end());
            // plot_mesh(ArrayShape{2000, 2000}, tl_street->get_triangles_around({-1.59931f, 0.321109f}, 0.01f), {}, {{-1.59931f, 0.321109f, 0.f}}).save_to_file("/tmp/plt.pgm");
            steiner_points = removed_duplicates(steiner_points, false);  // false = verbose
            BoundingInfo bounding_info{map_outer_contour, nodes, 0.1};
            LOG_INFO("add_street_steiner_points");
            add_street_steiner_points(
                steiner_points,
                hole_triangles,
                bounding_info,
                scale,
                steiner_point_distances_road,
                steiner_point_distances_steiner);
            LOG_INFO("triangulate_terrain_or_ceilings");
            triangulate_terrain_or_ceilings(
                *tl_terrain,
                bounding_info,
                steiner_points,
                map_outer_contour,
                hole_triangles,
                nodes,
                scale,
                uv_scale_terrain,
                0);
        }
        if (with_roofs) {
            LOG_INFO("draw_roofs");
            draw_roofs(
                tls_buildings,
                Material{
                    texture_descriptor: {color: roof_texture},
                    aggregate_mode: AggregateMode::ONCE,
                    ambience: {1, 1, 1}}.compute_color_mode(),
                roof_color,
                buildings,
                nodes,
                roof_width,
                scale,
                roof_height0,
                roof_height1);
        }
        if (with_ceilings) {
            LOG_INFO("draw_ceilings");
            draw_ceilings(
                tls_buildings,
                Material{
                    texture_descriptor: {color: ceiling_texture},
                    aggregate_mode: AggregateMode::ONCE,
                    ambience: {1, 1, 1}}.compute_color_mode(),
                buildings,
                nodes,
                scale,
                uv_scale_ceiling,
                max_wall_width);
        }
        if (remove_backfacing_triangles) {
            for(auto& l : tls_ground) {
                l->delete_backfacing_triangles();
            }
        }
    }

    auto tls_all = tls_ground;
    tls_all.insert(tls_all.end(), tls_buildings.begin(), tls_buildings.end());
    tls_all.insert(tls_all.end(), tls_wall_barriers.begin(), tls_wall_barriers.end());

    if (!heightmap.empty() || street_edge_smoothness > 0 || terrain_edge_smoothness > 0) {
        std::list<FixedArray<float, 3>*> smoothed_vertices;
        for(auto& l : tls_all) {
            for(auto& t : l->triangles_) {
                for(auto& v : t.flat_iterable()) {
                    smoothed_vertices.push_back(&v.position);
                }
            }
        }
        for(auto& d : object_resource_descriptors_) {
            smoothed_vertices.push_back(&d.position);
        }
        for(auto& i : resource_instance_positions_) {
            for(auto& d : i.second) {
                smoothed_vertices.push_back(&d.position);
            }
        }
        for(SteinerPointInfo& p : steiner_points) {
            smoothed_vertices.push_back(&p.position);
        }
        if (!heightmap.empty()) {
            LOG_INFO("apply_height_map");
            std::set<OrderableFixedArray<float, 2>> vertices_to_delete;
            apply_height_map(
                smoothed_vertices,
                vertices_to_delete,
                PgmImage::load_from_file(heightmap).to_float() / 64.f * float(UINT16_MAX),
                normalized_points.chained(ScaleMode::DIAGONAL, OffsetMode::MINIMUM).normalization_matrix(),
                scale,
                nodes,
                ways,
                height_bindings,
                street_node_smoothness);
            for(auto& l : tls_all) {
                std::remove_if(l->triangles_.begin(), l->triangles_.end(), [&vertices_to_delete](const FixedArray<ColoredVertex, 3>& v){
                    return
                        vertices_to_delete.contains(OrderableFixedArray<float, 2>{v(0).position(0), v(0).position(1)}) ||
                        vertices_to_delete.contains(OrderableFixedArray<float, 2>{v(1).position(0), v(1).position(1)}) ||
                        vertices_to_delete.contains(OrderableFixedArray<float, 2>{v(2).position(0), v(2).position(1)});
                });
            }
            std::remove_if(object_resource_descriptors_.begin(), object_resource_descriptors_.end(), [&vertices_to_delete](const ObjectResourceDescriptor& d){
                return vertices_to_delete.contains(OrderableFixedArray<float, 2>{d.position(0), d.position(1)});
            });
            for(auto& i : resource_instance_positions_) {
                std::remove_if(i.second.begin(), i.second.end(), [&vertices_to_delete](const ResourceInstanceDescriptor& d){
                    return vertices_to_delete.contains(OrderableFixedArray<float, 2>{d.position(0), d.position(1)});
                });
            }
            std::remove_if(steiner_points.begin(), steiner_points.end(), [&vertices_to_delete](const SteinerPointInfo& p){
                return vertices_to_delete.contains(OrderableFixedArray<float, 2>{p.position(0), p.position(1)});});
        }
        if (street_edge_smoothness > 0 || terrain_edge_smoothness > 0) {
            std::list<std::shared_ptr<TriangleList>> tls_street{tl_street_crossing, tl_path_crossing, tl_street, tl_path, tl_curb_street, tl_curb_path};
            if (street_edge_smoothness > 0) {
                LOG_INFO("smoothen_edges (street)");
                TriangleList::smoothen_edges(tls_street, {}, smoothed_vertices, street_edge_smoothness, 100);
            }
            if (terrain_edge_smoothness > 0) {
                LOG_INFO("smoothen_edges (ground)");
                TriangleList::smoothen_edges(tls_ground, tls_street, smoothed_vertices, terrain_edge_smoothness, 10);
            }
        }
    }
    raise_streets(
        *tl_street_crossing,
        *tl_path_crossing,
        *tl_street,
        *tl_path,
        *tl_curb_street,
        *tl_curb_path,
        *tl_terrain,
        scale,
        raise_streets_amount);
    if (extrude_curb_amount != 0) {
        tl_curb_street->extrude(extrude_curb_amount * scale, scale, uv_scale_street);
        tl_curb_path->extrude(extrude_curb_amount * scale, scale, uv_scale_street);
    }
    // Normals are invalid after "apply_height_map"
    for(auto& l : tls_ground) {
        l->calculate_triangle_normals();
    }

    TriangleList::convert_triangle_to_vertex_normals(tls_wall_barriers);
    TriangleList::convert_triangle_to_vertex_normals(tls_ground_wo_curb);

    // for(auto& l : tls_ground) {
    //     colorize_height_map(l->triangles_);
    // }

    for(const WaysideResourceNames& ws : waysides) {
        LOG_INFO("add_grass_on_steiner_points");
        ResourceNameCycle rnc{scene_node_resources, ws.resource_names};
        add_grass_on_steiner_points(
            resource_instance_positions_,
            object_resource_descriptors_,
            rnc,
            steiner_points,
            scale,
            ws.min_dist,
            ws.max_dist);
    }
    if (!grass_resource_names.empty()) {
        ResourceNameCycle rnc{scene_node_resources, grass_resource_names};
        LOG_INFO("add_grass_inside_triangles");
        add_grass_inside_triangles(
            resource_instance_positions_,
            object_resource_descriptors_,
            rnc,
            *tl_terrain,
            scale,
            much_grass_distance);
    }

    std::list<std::shared_ptr<ColoredVertexArray>> ts;
    for(auto& l : tls_all) {
        if (!l->triangles_.empty()) {
            ts.push_back(l->triangle_array());
        }
    }
    rva_ = std::make_shared<RenderableColoredVertexArray>(ts, nullptr, rendering_resources_);
}

void RenderableOsmMap::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter) const
{
    {
        size_t i = 0;
        for(auto& p : object_resource_descriptors_) {
            auto node = new SceneNode;
            node->set_position(p.position);
            node->set_scale(scale_ * p.scale);
            node->set_rotation({M_PI / 2, 0, 0});
            scene_node_resources_.instantiate_renderable(p.name, p.name, *node, SceneNodeResourceFilter{});
            if (node->requires_render_pass()) {
                scene_node.add_child(p.name + "-" + std::to_string(i++), node);
            } else {
                std::cerr << "Adding aggregate " << p.name << std::endl;
                scene_node.add_aggregate_child(p.name + "-" + std::to_string(i++), node);
            }
        }
    }
    for(auto& p : resource_instance_positions_) {
        auto node = new SceneNode;
        node->set_rotation({M_PI / 2, 0, 0});
        scene_node_resources_.instantiate_renderable(p.first, p.first, *node, SceneNodeResourceFilter{});
        if (node->requires_render_pass()) {
            throw std::runtime_error("Object " + p.first + " requires render pass");
        }
        scene_node.add_instances_child(p.first, node);
        for(const auto& r : p.second) {
            scene_node.add_instances_position(p.first, r.position);
        }
    }
    rva_->instantiate_renderable(name, scene_node, resource_filter);
}

std::list<std::shared_ptr<ColoredVertexArray>> RenderableOsmMap::get_triangle_meshes() const {
    return rva_->get_triangle_meshes();
}

void RenderableOsmMap::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    return rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}
