#pragma once
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>

namespace Mlib {

struct OsmTriangleLists;
template <class TPos>
class TriangleList;
template <class TData, size_t tshape0, size_t... tshape>
class OrderableFixedArray;
template <class TData, size_t... tshape>
class FixedArray;
struct Node;
struct Way;
struct SteinerPointInfo;
struct StreetRectangle;
template <class TData>
class NormalizedPointsFixed;
struct ObjectResourceDescriptor;
struct ResourceInstanceDescriptor;
enum class WayPointSandbox;
struct OsmResourceConfig;
class NodeHeightBinding;
template <class TPos>
class VertexHeightBinding;
struct StreetWayPoint;
class BatchResourceInstantiator;

enum class VertexOutOfHeightMapBehavior {
    THROW,
    DELETE
};

void smoothen_and_apply_heightmap(
    const OsmResourceConfig& config,
    const std::map<OrderableFixedArray<double, 2>, NodeHeightBinding>& node_height_bindings,
    std::map<const FixedArray<double, 3>*, VertexHeightBinding<double>>& vertex_height_bindings,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    const NormalizedPointsFixed<double>& normalized_points,
    const std::list<std::shared_ptr<TriangleList<double>>>& tls_buildings,
    const std::list<std::shared_ptr<TriangleList<double>>>& tls_wall_barriers,
    const OsmTriangleLists& osm_triangle_lists,
    const OsmTriangleLists& air_triangle_lists,
    VertexOutOfHeightMapBehavior vertex_out_of_height_map_behavior,
    BatchResourceInstantiator& bri,
    std::list<SteinerPointInfo>& steiner_points,
    std::list<FixedArray<double, 3>>& map_outer_contour3,
    std::list<StreetRectangle>& street_rectangles,
    std::map<WayPointSandbox, std::list<std::pair<StreetWayPoint, StreetWayPoint>>>& way_point_edge_descriptors);

}
