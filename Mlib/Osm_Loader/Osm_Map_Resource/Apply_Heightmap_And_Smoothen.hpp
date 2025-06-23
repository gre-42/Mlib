#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

namespace Mlib {

struct OsmTriangleLists;
template <class TPos>
class TriangleList;
template <class TData, size_t... tshape>
class OrderableFixedArray;
template <typename TData, size_t... tshape>
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
class StreetBvh;

enum class VertexOutOfHeightMapBehavior {
    THROW,
    DELETE
};

void apply_heightmap_and_smoothen(
    const OsmResourceConfig& config,
    const StreetBvh& ground_street_bvh,
    const StreetBvh& air_bvh,
    const std::map<OrderableFixedArray<CompressedScenePos, 2>, NodeHeightBinding>& node_height_bindings,
    std::unordered_map<FixedArray<CompressedScenePos, 3>*, VertexHeightBinding<CompressedScenePos>>& vertex_height_bindings,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    const NormalizedPointsFixed<ScenePos>& normalized_points,
    const std::list<std::shared_ptr<TriangleList<CompressedScenePos>>>& tls_wall_barriers,
    const OsmTriangleLists& osm_triangle_lists,
    const OsmTriangleLists& air_triangle_lists,
    VertexOutOfHeightMapBehavior vertex_out_of_height_map_behavior,
    BatchResourceInstantiator& bri,
    std::list<SteinerPointInfo>& steiner_points,
    std::list<FixedArray<CompressedScenePos, 3>>& map_outer_contour3,
    std::list<StreetRectangle>& street_rectangles,
    std::map<WayPointSandbox, std::list<std::pair<StreetWayPoint, StreetWayPoint>>>& way_point_edge_descriptors);

}
