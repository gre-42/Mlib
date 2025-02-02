#include "Record_Track.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Advance_Times/Rigid_Body_Recorder.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(filename);
}

const std::string RecordTrack::key = "record_track";

LoadSceneJsonUserFunction RecordTrack::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    RecordTrack(args.renderable_scene()).execute(args);
};

RecordTrack::RecordTrack(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void RecordTrack::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> recorder_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(recorder_node);
    auto& at = global_object_pool.create<RigidBodyRecorder>(
        CURRENT_SOURCE_LOCATION,
        args.arguments.path(KnownArgs::filename),
        scene_node_resources.get_geographic_mapping("world"),
        recorder_node,
        rb.rbp_,
        args.ui_focus.focuses);
    physics_engine.advance_times_.add_advance_time({ at, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
}
