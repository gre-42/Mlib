#pragma once
#include <Mlib/Geometry/Graph/Point_And_Flags.hpp>
#include <Mlib/Geometry/Graph/Points_And_Adjacency.hpp>
#include <Mlib/Map/Unordered_Map.hpp>
#include <Mlib/Map/Verbose_Unordered_Map.hpp>
#include <Mlib/Scene_Precision.hpp>

namespace Mlib {

struct WayPointsAndBvh;
enum class JoinedWayPointSandbox;
enum class WayPointLocation;
using PointsAndAdjacencyResource = PointsAndAdjacency<PointAndFlags<FixedArray<CompressedScenePos, 3>, WayPointLocation>>;
using WayPointSandboxes = UnorderedMap<JoinedWayPointSandbox, PointsAndAdjacencyResource>;
struct WayPointSandboxesAndBvh: public VerboseUnorderedMap<JoinedWayPointSandbox, std::shared_ptr<WayPointsAndBvh>>
{
    WayPointSandboxesAndBvh();
};

}
