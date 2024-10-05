#include "Set_Round_Mesh_Surface_Normal.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Collision/Detect/Round_Mesh_Normal.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(t);
DECLARE_ARGUMENT(k);
}

const std::string SetRoundMeshSurfaceNormal::key = "set_round_mesh_surface_normal";

LoadSceneJsonUserFunction SetRoundMeshSurfaceNormal::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetRoundMeshSurfaceNormal(args.renderable_scene()).execute(args);
};

SetRoundMeshSurfaceNormal::SetRoundMeshSurfaceNormal(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetRoundMeshSurfaceNormal::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    rb.set_surface_normal(std::make_unique<RoundMeshNormal>(
        args.arguments.at<float>(KnownArgs::t),
        args.arguments.at<float>(KnownArgs::k)));
}
