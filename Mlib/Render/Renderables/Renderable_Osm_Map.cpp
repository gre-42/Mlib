#include "Renderable_Osm_Map.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Normalized_Points_Fixed.hpp>
#include <Mlib/Images/PgmImage.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Geographic_Coordinates.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Render/Renderables/Renderable_Osm_Map_Helpers.hpp>
#include <Mlib/Scene_Graph/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/String.hpp>
#include <regex>

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
    float default_street_width,
    float roof_width,
    float scale,
    float uv_scale,
    float uv_scale_facade,
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
    bool with_terrain,
    bool with_buildings,
    bool only_raceways,
    float steiner_point_distance,
    float curb_alpha,
    float raise_streets_amount,
    bool add_street_lights,
    float max_wall_width,
    bool with_height_bindings)
: rendering_resources_{rendering_resources},
  scene_node_resources_{scene_node_resources},
  scale_{scale}
{
    std::ifstream ifs{filename};
    const std::regex node_reg("^ +<node id=[\"'](-?\\w+)[\"'] .*visible=[\"'](true|false)[\"'].* lat=[\"']([\\w.-]+)[\"'] lon=[\"']([\\w.-]+)[\"'].*>$");
    const std::regex way_reg("^ +<way id=[\"'](-?\\w+)[\"'].* visible=[\"'](true|false)[\"'].*>$");
    const std::regex way_end_reg("^ +</way>$");
    const std::regex node_ref_reg("^  +<nd ref=[\"'](-?\\w+)[\"'] */>$");
    const std::regex bounds_reg(" +<bounds minlat=[\"']([\\w.-]+)[\"'] minlon=[\"']([\\w.-]+)[\"'] maxlat=[\"']([\\w.-]+)[\"'] maxlon=[\"']([\\w.-]+)[\"'](?: origin=[\"'].*[\"'])? */>$");
    const std::regex tag_reg("  +<tag k=[\"']([\\w:.-]+)[\"'] v=[\"'](.*)[\"'] */>$");
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
        texture: terrain_texture,
        occluded_type: OccludedType::LIGHT_MAP_COLOR,
        occluder_type: OccluderType::WHITE,
        dirt_texture: dirt_texture,
        specularity: {0.2, 0.2, 0.2}});
    auto tl_street_crossing = std::make_shared<TriangleList>("street_crossing", Material{
        texture: asphalt_texture,
        occluded_type: OccludedType::LIGHT_MAP_COLOR,
        occluder_type: OccluderType::WHITE,
        specularity: {0.2, 0.2, 0.2}});
    auto tl_path_crossing = std::make_shared<TriangleList>("path_crossing", Material{
        texture: path_texture,
        occluded_type: OccludedType::LIGHT_MAP_COLOR,
        occluder_type: OccluderType::WHITE,
        specularity: {0.2, 0.2, 0.2}});
    auto tl_street = std::make_shared<TriangleList>("street", Material{
        texture: street_texture,
        occluded_type: OccludedType::LIGHT_MAP_COLOR,
        occluder_type: OccluderType::WHITE,
        specularity: {0.2, 0.2, 0.2}}); // mixed_texture: terrain_texture
    auto tl_path = std::make_shared<TriangleList>("path", Material{
        texture: path_texture,
        occluded_type: OccludedType::LIGHT_MAP_COLOR,
        occluder_type: OccluderType::WHITE,
        specularity: {0.2, 0.2, 0.2}}); // mixed_texture: terrain_texture
    auto tl_curb_street = std::make_shared<TriangleList>("curb_street", Material{
        texture: curb_street_texture,
        occluded_type: OccludedType::LIGHT_MAP_COLOR,
        occluder_type: OccluderType::WHITE,
        clamp_mode_s: ClampMode::EDGE,
        specularity: {0.2, 0.2, 0.2}}); // mixed_texture: terrain_texture
    auto tl_curb_path = std::make_shared<TriangleList>("curb_path", Material{
        texture: curb_path_texture,
        occluded_type: OccludedType::LIGHT_MAP_COLOR,
        occluder_type: OccluderType::WHITE,
        clamp_mode_s: ClampMode::EDGE,
        specularity: {0.2, 0.2, 0.2}}); // mixed_texture: terrain_texture
    std::list<std::shared_ptr<TriangleList>> tls_ground{tl_terrain, tl_street_crossing, tl_path_crossing, tl_street, tl_path, tl_curb_street, tl_curb_path};
    std::list<std::shared_ptr<TriangleList>> tls_buildings;
    std::list<std::shared_ptr<TriangleList>> tls_wall_barriers;
    std::map<OrderableFixedArray<float, 2>, std::set<std::string>> height_bindings;
    {
        std::vector<FixedArray<float, 2>> map_outer_contour;
        get_map_outer_contour(
            map_outer_contour,
            nodes,
            ways);

        std::list<FixedArray<float, 2>> steiner_points;
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
            resource_instance_positions_,
            height_bindings,
            nodes,
            ways,
            scale,
            uv_scale,
            default_street_width,
            only_raceways,
            curb_alpha,
            add_street_lights,
            with_height_bindings);

        if (forest_outline_tree_distance != INFINITY) {
            ResourceNameCycle rnc{tree_resource_names};
            add_trees_to_forest_outlines(
                resource_instance_positions_,
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
            //         clamp_mode: ClampMode::EDGE,
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
        if (with_tree_nodes) {
            ResourceNameCycle rnc{tree_resource_names};
            add_trees_to_tree_nodes(
                resource_instance_positions_,
                steiner_points,
                rnc,
                nodes,
                scale);
        }

        if (with_buildings) {
            draw_building_walls(
                tls_buildings,
                steiner_points,
                Material{
                    texture: "<tbd>",
                    aggregate_mode: AggregateMode::ONCE,
                    ambience: {1, 1, 1}},
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
            draw_building_walls(
                tls_wall_barriers,
                steiner_points,
                Material{
                    texture: "<tbd>",
                    occluder_type: OccluderType::OFF,
                    blend_mode: barrier_blend_mode,
                    aggregate_mode: AggregateMode::ONCE,
                    is_small: false,
                    cull_faces: false},
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
            triangulate_terrain_or_ceilings(
                *tl_terrain,
                steiner_points,
                map_outer_contour,
                hole_triangles,
                nodes,
                scale,
                uv_scale,
                0,
                steiner_point_distance);
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
        if (with_roofs) {
            draw_roofs(
                tls_buildings,
                Material{
                    texture: roof_texture,
                    aggregate_mode: AggregateMode::ONCE,
                    ambience: {1, 1, 1}},
                roof_color,
                buildings,
                nodes,
                roof_width,
                scale,
                roof_height0,
                roof_height1);
        }
        if (with_ceilings) {
            draw_ceilings(
                tls_buildings,
                Material{
                    texture: ceiling_texture,
                    aggregate_mode: AggregateMode::ONCE,
                    ambience: {1, 1, 1}},
                buildings,
                nodes,
                scale,
                uv_scale,
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

    if (!heightmap.empty()) {
        apply_height_map(
            tls_all,
            resource_instance_positions_,
            PgmImage::load_from_file(heightmap).to_float() / 64.f * float(UINT16_MAX),
            normalized_points.chained(ScaleMode::DIAGONAL, OffsetMode::MINIMUM).normalization_matrix(),
            scale,
            nodes,
            height_bindings);
    }

    // for(auto& l : tls_buildings) {
    //     colorize_height_map(l->triangles_);
    // }
    TriangleList::convert_triangle_to_vertex_normals(tls_wall_barriers);
    // for(auto& l : tls_wall_barriers) {
    //     colorize_height_map(l->triangles_);
    // }

    // Normals are invalid after "apply_height_map"
    for(auto& l : tls_ground) {
        l->calculate_triangle_normals();
    }

    TriangleList::convert_triangle_to_vertex_normals(tls_ground);

    // for(auto& l : tls_ground) {
    //     colorize_height_map(l->triangles_);
    // }

    if (much_grass_distance != INFINITY) {
        ResourceNameCycle rnc{grass_resource_names};
        add_grass_inside_triangles(
            resource_instance_positions_,
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

void RenderableOsmMap::instantiate_renderable(const std::string& name, SceneNode& scene_node, const SceneNodeResourceFilter& resource_filter)
{
    {
        size_t i = 0;
        for(auto& p : resource_instance_positions_) {
            auto grass_node = new SceneNode;
            grass_node->set_position(p.position);
            grass_node->set_scale(scale_ * p.scale);
            grass_node->set_rotation({M_PI / 2, 0, 0});
            scene_node_resources_.instantiate_renderable(p.name, p.name, *grass_node, SceneNodeResourceFilter{});
            if (grass_node->requires_render_pass()) {
                scene_node.add_child(p.name + "-" + std::to_string(i++), grass_node);
            } else {
                scene_node.add_instances_child(p.name + "-" + std::to_string(i++), grass_node);
            }
        }
    }
    rva_->instantiate_renderable(name, scene_node, resource_filter);
}

std::list<std::shared_ptr<ColoredVertexArray>> RenderableOsmMap::get_triangle_meshes() {
    return rva_->get_triangle_meshes();
}

void RenderableOsmMap::generate_triangle_rays(size_t npoints, const FixedArray<float, 3>& lengths, bool delete_triangles) {
    return rva_->generate_triangle_rays(npoints, lengths, delete_triangles);
}
