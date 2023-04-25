#include "Record_Track_Gpx.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Advance_Times/Rigid_Body_Recorder_Gpx.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
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
    RecordTrackGpx(args.renderable_scene()).execute(args);
};

RecordTrackGpx::RecordTrackGpx(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void RecordTrackGpx::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto& recorder_node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node));
    auto rb = dynamic_cast<RigidBodyVehicle*>(&recorder_node.get_absolute_movable());
    if (rb == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a rigid body");
    }
    physics_engine.advance_times_.add_advance_time(std::make_unique<RigidBodyRecorderGpx>(
        args.arguments.path(KnownArgs::filename),
        physics_engine.advance_times_,
        recorder_node,
        &rb->rbi_,
        scene_node_resources.get_geographic_mapping("world"),
        args.ui_focus.focuses));
}
