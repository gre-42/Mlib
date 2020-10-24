#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Aggregate_Mode.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <list>
#include <map>
#include <set>

namespace Mlib {

template <class TData, size_t... tshape>
class OrderableFixedArray;
class SceneNodeResources;
struct ColoredVertex;
struct Material;
struct ResourceInstanceDescriptor;
struct ObjectResourceDescriptor;
struct TriangleList;

static const FixedArray<float, 3> way_color{1, 1, 1};      // replaced with texture
static const FixedArray<float, 3> terrain_color{1, 1, 1};  // replaced with texture
static const FixedArray<float, 3> building_color{1, 1, 1};
static const FixedArray<float, 3> roof_color{1, 1, 1};
static const float roof_height0 = 5;
static const float roof_height1 = 9;
// grep highway map.osm | grep -v pedestrian | grep -v path | grep -v footway | grep -v cycleway
static const std::set<std::string> excluded_highways = {"pedestrian", "path", "footway", "cycleway", "steps"};
// static const std::set<std::string> path_tags = {"track", "tertiary"};
static const std::set<std::string> excluded_highway_tags = {"layer"};
static const std::set<std::string> included_barriers = {"wall", "guard_rail"};

struct Node {
    FixedArray<float, 2> position;
    std::map<std::string, std::string> tags;
};

struct Way {
    std::list<std::string> nd;
    std::map<std::string, std::string> tags;
};

void draw_node(
    std::vector<FixedArray<ColoredVertex, 3>>& triangles,
    const FixedArray<float, 2>& pos2d,
    float size = 0.01);

void draw_nodes(
    std::vector<FixedArray<ColoredVertex, 3>>& triangles,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, std::list<std::string>>& ways);

// void draw_test_lines(
//     TriangleList& tl,
//     float width);

float parse_meters(const std::map<std::string, std::string>& tags, const std::string& key, float default_value);

struct Building {
    std::string id;
    const Way& way;
    float building_top;
    float building_bottom;
    float area = 0;
};

enum class BuildingType {
    BUILDING,
    WALL_BARRIER
};

struct ParsedResourceName {
    std::string name;
    float probability;
    AggregateMode aggregate_mode;
};

class ResourceNameCycle {
public:
    ResourceNameCycle(const SceneNodeResources& resources, const std::vector<std::string>& names);
    const ParsedResourceName& operator () ();
private:
    std::vector<ParsedResourceName> names_;
    UniformIntRandomNumberGenerator<size_t> rng0_;
    UniformRandomNumberGenerator<float> rng_;
};

std::list<Building> get_buildings_or_wall_barriers(
    BuildingType building_type,
    const std::map<std::string, Way>& ways,
    float building_bottom,
    float default_building_top);

void draw_roofs(
    std::list<std::shared_ptr<TriangleList>>& tls,
    const Material& material,
    const FixedArray<float, 3>& color,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float width,
    float scale,
    float z0,
    float z1);

void draw_ceilings(
    std::list<std::shared_ptr<TriangleList>>& tls,
    const Material& material,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_width);

enum class RoadType {
    STREET,
    PATH
};

struct AngleWay {
    std::string id;
    float width;
    RoadType road_type;
};

struct NeighborWay {
    float angle;
    float width;
};

struct NodeWayInfo {
    float way_length;
    FixedArray<float, 3> color;
};

void get_neighbors(
    const std::string& center,
    const std::map<std::string, NeighborWay>& neighbors,
    const std::map<float, AngleWay>& angles,
    const std::string** l,
    const std::string** r);

//class PolygonDrawer {
//public:
//    void draw_line(const p2t::Point& from, const p2t::Point& to, size_t nsteps) {
//        for(size_t i = 0; i < nsteps; ++i) {
//            double alpha = double(i) / nsteps;
//            point_list_.push_back((1 - alpha) * from + alpha * to);
//            points_.push_back(&point_list_.back());
//        }
//    }
//    std::vector<p2t::Point*> points_;
//private:
//    std::list<p2t::Point> point_list_;
//};

void get_map_outer_contour(
    std::vector<FixedArray<float, 2>>& contour,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways);

void draw_streets(
    TriangleList& tl_street_crossing,
    TriangleList& tl_path_crossing,
    TriangleList& tl_street,
    TriangleList& tl_path,
    TriangleList& tl_curb_street,
    TriangleList& tl_curb_path,
    std::list<ObjectResourceDescriptor>& street_light_positions,
    std::map<OrderableFixedArray<float, 2>, std::set<std::string>>& height_bindings,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    float scale,
    float uv_scale,
    float default_street_width,
    bool only_raceways,
    const std::string& name_pattern,
    const std::set<std::string>& path_tags,
    float curb_alpha,
    bool add_street_lights,
    bool with_height_bindings);

void raise_streets(
    TriangleList& tl_street_crossing,
    TriangleList& tl_path_crossing,
    TriangleList& tl_street,
    TriangleList& tl_path,
    TriangleList& tl_curb_street,
    TriangleList& tl_curb_path,
    TriangleList& tl_terrain,
    float scale,
    float amount);

void triangulate_terrain_or_ceilings(
    TriangleList& tl_terrain,
    const std::list<FixedArray<float, 2>>& steiner_points,
    const std::vector<FixedArray<float, 2>>& bounding_contour,
    const std::list<FixedArray<ColoredVertex, 3>>& hole_triangles,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float z,
    float steiner_point_distance);

void apply_height_map(
    std::list<std::shared_ptr<TriangleList>>& triangles,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    const Array<float>& heightmap,
    const FixedArray<float, 2, 3>& normalization_matrix,
    float scale,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    const std::map<OrderableFixedArray<float, 2>, std::set<std::string>>& height_bindings);

void add_grass_inside_triangles(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& fern_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    ResourceNameCycle& rnc,
    const TriangleList& triangles,
    float scale,
    float distance);

void add_trees_to_forest_outlines(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& fern_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::list<FixedArray<float, 2>>& steiner_points,
    ResourceNameCycle& rnc,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    float tree_distance,
    float tree_inwards_distance,
    float scale);

void add_beacons_to_raceways(
    std::list<ObjectResourceDescriptor>& street_light_positions,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    float raceway_beacon_distance,
    float scale);

// void add_grass_outlines(
//     std::map<std::string, std::list<ResourceInstanceDescriptor>>& fern_positions,
//     std::list<FixedArray<float, 2>>& steiner_points,
//     const std::map<std::string, Node>& nodes,
//     const std::map<std::string, Way>& ways,
//     bool continuous,
//     float tree_distance,
//     float tree_inwards_distance,
//     float scale);

void add_trees_to_tree_nodes(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& fern_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::list<FixedArray<float, 2>>& steiner_points,
    ResourceNameCycle& rnc,
    const std::map<std::string, Node>& nodes,
    float scale);

void add_binary_vegetation_old(
    std::list<std::shared_ptr<TriangleList>>& tls,
    const Material& material,
    const std::string& grass_texture,
    const std::string& tree_texture,
    const std::string& tree_texture_2,
    const TriangleList& ground_triangles,
    float scale);

/*enum class TriangleMaterial {
    ROAD,
    TERRAIN
};

template <class TTag>
class TriangleTag {
    using O = OrderableFixedArray<float, 3>;
public:
    TTag get(const FixedArray<ColoredVertex, 3>& triangle) const {
        OrderableFixedArray<O, 3> key;
        key(0) = O{triangle(0).position};
        key(1) = O{triangle(1).position};
        key(2) = O{triangle(2).position};
        return tags_.at(key);
    }
    void set(const FixedArray<ColoredVertex, 3>& triangle, const TTag& tag) {
        OrderableFixedArray<OrderableFixedArray<float, 3>, 3> key{
            triangle(0).position,
            triangle(1).position,
            triangle(2).position};
        auto it = tags_.find(key);
        if (it != tags_.end()) {
            throw std::runtime_error("Tag already set");
            tags_.insert(key, tag);
        }
    }
private:
    std::map<OrderableFixedArray<OrderableFixedArray<float, 3>, 3>, TriangleMaterial> tags_;
};*/

void colorize_height_map(std::list<FixedArray<ColoredVertex, 3>>& triangles);

void compute_building_area(
    std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale);

void draw_building_walls(
    std::list<std::shared_ptr<TriangleList>>& tls,
    std::list<FixedArray<float, 2>>& steiner_points,
    const Material& material,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_width,
    const std::string& facade_texture,
    const std::string& facade_texture_2,
    const std::string& facade_texture_3);

std::vector<FixedArray<float, 2>> removed_duplicates(const std::vector<FixedArray<float, 2>>& nodes, bool verbose = true);
std::list<FixedArray<float, 2>> removed_duplicates(const std::list<FixedArray<float, 2>>& nodes, bool verbose = true);

}
