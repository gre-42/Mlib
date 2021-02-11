#pragma once
#include <list>
#include <map>
#include <set>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <typename TData, size_t... tshape>
class OrderableFixedArray;
class TriangleList;
struct ResourceInstanceDescriptor;
struct StreetRectangle;
struct ObjectResourceDescriptor;
class ResourceNameCycle;
enum class DrivingDirection;
struct Node;
struct Way;
enum class WayPointLocation;
struct AngleWay;
struct NeighborWay;
struct AngleCurb;
struct HoleWaypoint;
struct NodeWayInfo;
struct OsmTriangleLists;

struct DrawStreetsInput {
    OsmTriangleLists& ground_triangles;
    OsmTriangleLists& air_triangles;
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions;
    std::list<ObjectResourceDescriptor>& object_resource_descriptors;
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes;
    std::list<StreetRectangle>& street_rectangles;
    std::map<OrderableFixedArray<float, 2>, std::set<std::string>>& height_bindings;
    std::list<std::pair<std::string, std::string>>& way_point_edges_1_lane_street;
    std::map<WayPointLocation, std::list<std::pair<FixedArray<float, 3>, FixedArray<float, 3>>>>& way_point_edges_2_lanes;
    const std::map<std::string, Node>& nodes;
    const std::map<std::string, Way>& ways;
    float scale;
    float uv_scale;
    float default_street_width;
    bool only_raceways;
    const std::string& name_pattern;
    const std::set<std::string>& excluded_highways;
    const std::set<std::string>& path_tags;
    float curb_alpha;
    float curb2_alpha;
    float curb_uv_x;
    float curb2_uv_x;
    ResourceNameCycle& street_lights;
    bool with_height_bindings;
    DrivingDirection driving_direction;
};

class DrawStreets: private DrawStreetsInput {
public:
    explicit DrawStreets(const DrawStreetsInput& in);
    ~DrawStreets();
private:
    void initialize_arrays();
    void calculate_neighbors();
    void draw_streets();
    void draw_holes();
    std::map<std::string, std::map<float, AngleWay>> node_angles;
    std::map<std::string, std::map<std::string, NeighborWay>> node_neighbors;
    std::map<std::string, std::map<AngleCurb, FixedArray<float, 2>>> node_hole_contours;
    std::map<std::string, HoleWaypoint> node_hole_waypoints_street;
    std::map<std::string, HoleWaypoint> node_hole_waypoints_sidewalk;
    std::map<std::string, NodeWayInfo> node_way_info;
};

}
