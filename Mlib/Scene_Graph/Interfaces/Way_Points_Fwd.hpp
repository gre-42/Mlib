#include <Mlib/Geometry/Mesh/Point_And_Flags.hpp>
#include <Mlib/Geometry/Mesh/Points_And_Adjacency.hpp>
#include <Mlib/Map/Unordered_Map.hpp>
#include <Mlib/Scene_Precision.hpp>

namespace Mlib {

struct WayPointsAndBvh;
enum class JoinedWayPointSandbox;
enum class WayPointLocation;
using PointsAndAdjacencyResource = PointsAndAdjacency<PointAndFlags<FixedArray<CompressedScenePos, 3>, WayPointLocation>>;
using WayPointSandboxes = UnorderedMap<JoinedWayPointSandbox, PointsAndAdjacencyResource>;
using WayPointSandboxesAndBvh = UnorderedMap<JoinedWayPointSandbox, std::shared_ptr<WayPointsAndBvh>>;

}
