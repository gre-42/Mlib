#include "Way_Points_Fwd.hpp"
#include <Mlib/Scene_Graph/Joined_Way_Point_Sandbox.hpp>

using namespace Mlib;

WayPointSandboxesAndBvh::WayPointSandboxesAndBvh()
    : VerboseUnorderedMap<JoinedWayPointSandbox, std::shared_ptr<WayPointsAndBvh>>{
        "Waypoint sandbox and BVH", [](const JoinedWayPointSandbox key){ return joined_way_point_sandbox_to_string(key); }
    }
{}
