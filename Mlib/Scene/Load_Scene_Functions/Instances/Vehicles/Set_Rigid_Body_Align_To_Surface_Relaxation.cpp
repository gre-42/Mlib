#include "Set_Rigid_Body_Align_To_Surface_Relaxation.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(value);
}

const std::string SetRigidBodyAlignToSurfaceRelaxation::key = "set_rigid_body_align_to_surface_relaxation";

LoadSceneJsonUserFunction SetRigidBodyAlignToSurfaceRelaxation::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetRigidBodyAlignToSurfaceRelaxation(args.renderable_scene()).execute(args);
};

SetRigidBodyAlignToSurfaceRelaxation::SetRigidBodyAlignToSurfaceRelaxation(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetRigidBodyAlignToSurfaceRelaxation::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    rb.align_to_surface_state_.align_to_surface_relaxation_ = args.arguments.at<float>(KnownArgs::value);
}
