#include "Set_Waypoint_Ofs.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(vehicle);
DECLARE_ARGUMENT(dy);
}

const std::string SetWaypointOfs::key = "set_waypoint_ofs";

LoadSceneJsonUserFunction SetWaypointOfs::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetWaypointOfs(args.renderable_scene()).execute(args);
};

SetWaypointOfs::SetWaypointOfs(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetWaypointOfs::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::vehicle), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    rb.set_waypoint_ofs(CompressedScenePos::from_float_safe(args.arguments.at<ScenePos>(KnownArgs::dy) * meters));
}
