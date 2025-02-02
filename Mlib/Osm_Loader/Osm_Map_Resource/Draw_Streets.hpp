#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Map/Map.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TData, size_t... tshape>
class OrderableFixedArray;
class SceneNodeResources;
template <class TPos>
class TriangleList;
struct ResourceInstanceDescriptor;
struct StreetRectangle;
struct ObjectResourceDescriptor;
class ResourceNameCycle;
enum class DrivingDirection;
struct Node;
struct Way;
enum class WayPointSandbox;
struct AngleWay;
struct NeighborWay;
struct AngleCurb;
struct HoleWaypoint;
struct NodeWayInfo;
struct OsmTriangleLists;
struct OsmRectangle2D;
template <class TPos>
struct ColoredVertex;
struct WayInfo;
struct NodeHoleVertex;
class NodeHeightBinding;
enum class RoadType;
template <class TPos>
class ColoredVertexArray;
struct StreetWayPoint;
class BatchResourceInstantiator;
class RacingLineBvh;

struct DrawStreetsInput {
    SceneNodeResources& scene_node_resources;
    OsmTriangleLists& ground_triangles;
    OsmTriangleLists& air_triangles;
    BatchResourceInstantiator& bri;
    std::list<StreetRectangle>& street_rectangles;
    std::map<OrderableFixedArray<CompressedScenePos, 2>, NodeHeightBinding>& node_height_bindings;
    std::map<WayPointSandbox, std::list<std::pair<StreetWayPoint, StreetWayPoint>>>& way_point_edge_descriptors;
    UUVector<FixedArray<ColoredVertex<float>, 3>>& tunnel_pipe_triangles;
    UUVector<FixedArray<ColoredVertex<float>, 3>>& tunnel_bdry_triangles;
    std::list<FixedArray<CompressedScenePos, 2, 2>>& way_segments;
    const RacingLineBvh& racing_line_bvh;
    const Map<RoadType, std::string>& street_surface_central_resource_names;
    const Map<RoadType, std::string>& street_surface_endpoint0_resource_names;
    const Map<RoadType, std::string>& street_surface_endpoint1_resource_names;
    const Map<RoadType, std::string>& street_bumps_central_resource_names;
    const Map<RoadType, std::string>& street_bumps_endpoint0_resource_names;
    const Map<RoadType, std::string>& street_bumps_endpoint1_resource_names;
    const std::map<std::string, Node>& nodes;
    const std::map<std::string, Way>& ways;
    float scale;
    std::map<RoadType, float> uv_scales;
    float uv_scale_crossings;
    float default_street_width;
    float default_lane_width;
    float default_tunnel_pipe_width;
    CompressedScenePos default_tunnel_pipe_height;
    bool only_raceways_and_walls;
    const std::string& name_pattern;
    const std::set<std::string>& excluded_highways;
    const std::set<std::string>& path_tags;
    const std::set<std::string>& included_aeroways;
    float curb_alpha_;
    float curb2_alpha_;
    FixedArray<float, 2> curb_uv = fixed_nans<float, 2>();
    FixedArray<float, 2> curb2_uv = fixed_nans<float, 2>();
    FixedArray<float, 3> curb_color_ = fixed_nans<float, 3>();
    float racing_line_width_x;
    float racing_line_scale_y;
    ResourceNameCycle& street_lights;
    bool with_height_bindings;
    DrivingDirection driving_direction;
    Interp<double> layer_heights;
    bool use_terrain_holes;
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
    void draw_streets_add_waypoints(
        const OsmRectangle2D& rect,
        float curb_alpha,
        float curb2_alpha,
        unsigned int nlanes,
        float lane_shift,
        const std::string& node_id,
        const AngleWay& angle_way);
    void draw_streets_draw_ways(
        const OsmRectangle2D& rect,
        const std::string& node_id,
        const AngleWay& angle_way);
    void draw_streets_find_hole_contours(
        const OsmRectangle2D& rect,
        const std::string& node_id,
        const AngleWay& angle_way,
        float node_angle);
    void draw_streets_find_hole_waypoints(
        const OsmRectangle2D& rect,
        const std::string& node_id,
        const AngleWay& angle_way,
        float curb_alpha,
        float curb2_alpha,
        float lane_shift);
    std::string auto_model_name(
        const std::string& node_id,
        const AngleWay& angle_way,
        const Map<RoadType, std::string>& central_resource_names,
        const Map<RoadType, std::string>& endpoint0_resource_names,
        const Map<RoadType, std::string>& endpoint1_resource_names) const;
    std::map<std::string, WayInfo> way_infos;
    std::map<std::string, std::map<float, AngleWay>> node_angles;
    std::map<std::string, std::map<std::string, NeighborWay>> node_neighbors;
    std::map<std::string, std::map<AngleCurb, NodeHoleVertex>> node_hole_contours;
    std::map<std::string, std::map<AngleCurb, NodeHoleVertex>> air_support_node_hole_contours;
    std::map<std::string, std::map<AngleCurb, NodeHoleVertex>> tunnel_node_hole_contours;
    std::map<WayPointSandbox, std::map<std::string, HoleWaypoint>> node_hole_waypoints;
    std::map<std::string, NodeWayInfo> node_way_info;
};

}
