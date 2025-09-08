#include "Record_Track_Gpx.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Advance_Times/Rigid_Body_Recorder_Gpx.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(filename);
}

const std::string RecordTrackGpx::key = "record_track_gpx";

LoadSceneJsonUserFunction RecordTrackGpx::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    RecordTrackGpx(args.physics_scene()).execute(args);
};

RecordTrackGpx::RecordTrackGpx(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void RecordTrackGpx::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> recorder_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(recorder_node);
    auto& at = global_object_pool.create<RigidBodyRecorderGpx>(
        CURRENT_SOURCE_LOCATION,
        args.arguments.path(KnownArgs::filename),
        recorder_node,
        rb.rbp_,
        scene_node_resources.get_geographic_mapping(VariableAndHash<std::string>{"world"}),
        &countdown_start);
    physics_engine.advance_times_.add_advance_time({ at, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
}
