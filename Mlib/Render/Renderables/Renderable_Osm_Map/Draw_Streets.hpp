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
struct TriangleList;
struct ResourceInstanceDescriptor;
struct StreetRectangle;
struct ObjectResourceDescriptor;
struct ResourceNameCycle;
enum class DrivingDirection;
struct Node;
struct Way;
enum class WayPointLocation;

void draw_streets(
    TriangleList& tl_street_crossing,
    TriangleList& tl_path_crossing,
    TriangleList& tl_street,
    TriangleList& tl_path,
    TriangleList& tl_curb_street,
    TriangleList& tl_curb_path,
    TriangleList& tl_curb2_street,
    TriangleList& tl_curb2_path,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& street_light_positions,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    std::list<StreetRectangle>& street_rectangles,
    std::map<OrderableFixedArray<float, 2>, std::set<std::string>>& height_bindings,
    std::list<std::pair<std::string, std::string>>& way_point_edges_1_lane,
    std::map<WayPointLocation, std::list<std::pair<FixedArray<float, 3>, FixedArray<float, 3>>>>& way_point_edges_2_lanes,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    float scale,
    float uv_scale,
    float default_street_width,
    bool only_raceways,
    const std::string& name_pattern,
    const std::set<std::string>& excluded_highways,
    const std::set<std::string>& path_tags,
    float curb_alpha,
    float curb2_alpha,
    float curb_uv_x,
    float curb2_uv_x,
    ResourceNameCycle& street_lights,
    bool with_height_bindings,
    DrivingDirection driving_direction);

}
