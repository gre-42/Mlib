#include "Set_Capsule_Surface_Normal.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Collision/Detect/Normal_On_Capsule.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(rotation);
DECLARE_ARGUMENT(position);
}

const std::string SetCapsuleSurfaceNormal::key = "set_capsule_surface_normal";

LoadSceneJsonUserFunction SetCapsuleSurfaceNormal::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetCapsuleSurfaceNormal(args.renderable_scene()).execute(args);
};

SetCapsuleSurfaceNormal::SetCapsuleSurfaceNormal(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetCapsuleSurfaceNormal::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    FixedArray<float, 3, 3> R{ args.arguments.at<UFixedArray<UFixedArray<float, 3>, 3>>(KnownArgs::rotation) };
    rb.set_surface_normal(std::make_unique<NormalOnCapsule>(
        rb.rbp_,
        TransformationMatrix<float, ScenePos, 3>{
            R,
            args.arguments.at<UFixedArray<ScenePos, 3>>(KnownArgs::position) * (ScenePos)meters}));
}
