#include "Record_Track.hpp"
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Focus.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Physics/Advance_Times/Rigid_Body_Recorder.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Resources/Scene_Node_Resources.hpp>
#include <stdexcept>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(filename);
}

RecordTrack::RecordTrack(PhysicsScene& physics_scene)
    : LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void RecordTrack::execute(const LoadSceneJsonUserFunctionArgs& args) {
    args.arguments.validate(KnownArgs::options);
    DanglingBaseClassRef<SceneNode> recorder_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), CURRENT_SOURCE_LOCATION);
    auto rb = get_rigid_body_vehicle(recorder_node.get(), CURRENT_SOURCE_LOCATION);
    auto& at = global_object_pool.create<RigidBodyRecorder>(
        CURRENT_SOURCE_LOCATION,
        args.arguments.path(KnownArgs::filename),
        scene_node_resources.get_geographic_mapping(VariableAndHash<std::string>{"world"}),
        recorder_node,
        rb,
        &countdown_start);
    physics_engine.advance_times_.add_advance_time({ at, CURRENT_SOURCE_LOCATION }, CURRENT_SOURCE_LOCATION);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "record_track",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                RecordTrack{args.physics_scene()}.execute(args);
            });
    }
} obj;

}
