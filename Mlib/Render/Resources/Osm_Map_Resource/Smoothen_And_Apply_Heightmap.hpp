#include <list>
#include <map>
#include <memory>
#include <set>

namespace Mlib {

struct OsmTriangleLists;
class TriangleList;
template <class TData, size_t... tshape>
class OrderableFixedArray;
template <class TData, size_t... tshape>
class FixedArray;
struct Node;
struct Way;
struct SteinerPointInfo;
struct StreetRectangle;
class NormalizedPointsFixed;
struct ObjectResourceDescriptor;
struct ResourceInstanceDescriptor;
enum class WayPointLocation;
struct OsmResourceConfig;

void smoothen_and_apply_heightmap(
    const OsmResourceConfig& config,
    const std::map<OrderableFixedArray<float, 2>, std::set<std::string>>& height_bindings,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    const NormalizedPointsFixed& normalized_points,
    const std::list<std::shared_ptr<TriangleList>>& tls_buildings,
    const std::list<std::shared_ptr<TriangleList>>& tls_wall_barriers,
    const OsmTriangleLists& osm_triangle_lists,
    const OsmTriangleLists& air_triangle_lists,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    std::list<SteinerPointInfo>& steiner_points,
    std::list<StreetRectangle>& street_rectangles,
    std::map<WayPointLocation, std::list<std::pair<FixedArray<float, 3>, FixedArray<float, 3>>>>& way_point_edges_2_lanes);

}
