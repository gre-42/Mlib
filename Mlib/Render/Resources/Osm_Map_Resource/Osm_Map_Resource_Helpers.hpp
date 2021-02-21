#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Aggregate_Mode.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <list>
#include <map>
#include <set>

namespace Mlib {

template <class TData, size_t n>
class TransformationMatrix;
template <class TData, size_t... tshape>
class OrderableFixedArray;
class SceneNodeResources;
struct ColoredVertex;
struct Material;
struct ResourceInstanceDescriptor;
struct ObjectResourceDescriptor;
class TriangleList;
struct ParsedResourceName;
enum class DrivingDirection;
enum class WayPointLocation;
template <class TData>
class Interp;
struct OsmTriangleLists;

static const FixedArray<float, 3> way_color{1.f, 1.f, 1.f };      // replaced with texture
static const FixedArray<float, 3> terrain_color{1.f, 1.f, 1.f };  // replaced with texture
static const FixedArray<float, 3> building_color{1.f, 1.f, 1.f };
static const FixedArray<float, 3> roof_color{1.f, 1.f, 1.f };
static const float roof_height0 = 5;
static const float roof_height1 = 9;
// grep highway map.osm | grep -v pedestrian | grep -v path | grep -v footway | grep -v cycleway
// static const std::set<std::string> excluded_highways = {"pedestrian", "path", "footway", "cycleway", "steps"};
// static const std::set<std::string> path_tags = {"track", "tertiary"};
static const std::set<std::string> excluded_highway_tags = {};
static const std::set<std::string> included_barriers = {"wall", "guard_rail"};
static const std::set<std::string> excluded_buildings = {"roof"};

struct Node {
    FixedArray<float, 2> position;
    std::map<std::string, std::string> tags;
};

struct Way {
    std::list<std::string> nd;
    std::map<std::string, std::string> tags;
};

enum class SteinerPointType {
    STREET_NEIGHBOR,
    FOREST_OUTLINE,
    WALL,
    TREE_NODE
};

struct SteinerPointInfo {
    FixedArray<float, 3> position;
    SteinerPointType type;
    float distance_to_road;
};

struct StreetRectangle {
    WayPointLocation location;
    size_t nlanes;
    FixedArray<FixedArray<float, 3>, 2, 2> rectangle;
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
    WALL_BARRIER,
    SPAWN_LINE,
    WAYPOINTS
};

class ResourceNameCycle {
public:
    ResourceNameCycle(
        const SceneNodeResources& resources,
        const std::vector<std::string>& names);
    ~ResourceNameCycle();
    const ParsedResourceName* try_once();
    const ParsedResourceName& operator () ();
    bool empty() const;
    void seed(unsigned int seed);
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

//class PolygonDrawer {
//public:
//    void draw_line(const p2t::Point& from, const p2t::Point& to, size_t nsteps) {
//        for (size_t i = 0; i < nsteps; ++i) {
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

void raise_streets(
    const std::list<std::shared_ptr<TriangleList>>& tls_street_wo_curb,
    const std::list<std::shared_ptr<TriangleList>>& tls_ground,
    float scale,
    float amount);

struct BoundingInfo {
    BoundingInfo(
        const std::vector<FixedArray<float, 2>>& bounding_contour,
        const std::map<std::string, Node>& nodes,
        float border_width);
    FixedArray<float, 2> boundary_min;
    FixedArray<float, 2> boundary_max;
    float border_width;
};

void add_street_steiner_points(
    std::list<SteinerPointInfo>& steiner_points,
    const std::list<FixedArray<ColoredVertex, 3>>& triangles,
    const BoundingInfo& bounding_info,
    float scale,
    const std::vector<float>& steiner_point_distances_road,
    const std::vector<float>& steiner_point_distances_steiner);

void triangulate_terrain_or_ceilings(
    TriangleList& tl_terrain,
    TriangleList* tl_terrain_visuals,
    const std::list<std::list<FixedArray<ColoredVertex, 3>>>& tl_insert,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const std::vector<FixedArray<float, 2>>& bounding_contour,
    const std::list<FixedArray<ColoredVertex, 3>>& hole_triangles,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float z);

void apply_height_map(
    const TriangleList& tl_terrain,
    const std::set<OrderableFixedArray<float, 2>>& tunnel_entrances,
    std::list<FixedArray<float, 3>*>& in_vertices,
    std::set<const FixedArray<float, 3>*>& vertices_to_delete,
    const Array<float>& heightmap,
    const TransformationMatrix<float, 2>& normalization_matrix,
    float scale,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    const std::map<OrderableFixedArray<float, 2>, std::set<std::string>>& height_bindings,
    float street_node_smoothness,
    const Interp<float>& layer_height);

void add_grass_on_steiner_points(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    ResourceNameCycle& rnc,
    const std::list<SteinerPointInfo>& steiner_points,
    float scale,
    float dmin,
    float dmax);

void add_grass_inside_triangles(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    ResourceNameCycle& rnc,
    const TriangleList& triangles,
    float scale,
    float distance);

void add_trees_to_forest_outlines(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    std::list<SteinerPointInfo>& steiner_points,
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
//     std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
//     std::list<FixedArray<float, 2>>& steiner_points,
//     const std::map<std::string, Node>& nodes,
//     const std::map<std::string, Way>& ways,
//     bool continuous,
//     float tree_distance,
//     float tree_inwards_distance,
//     float scale);

void add_trees_to_tree_nodes(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    std::list<SteinerPointInfo>& steiner_points,
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
    std::list<SteinerPointInfo>& steiner_points,
    const Material& material,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_width,
    const std::vector<std::string>& facade_textures);

std::vector<FixedArray<float, 2>> removed_duplicates(const std::vector<FixedArray<float, 2>>& nodes, bool verbose = true);
std::list<FixedArray<float, 2>> removed_duplicates(const std::list<FixedArray<float, 2>>& nodes, bool verbose = true);
std::list<SteinerPointInfo> removed_duplicates(const std::list<SteinerPointInfo>& nodes, bool verbose = true);

void check_curb_validity(float curb_alpha, float curb2_alpha);

}
