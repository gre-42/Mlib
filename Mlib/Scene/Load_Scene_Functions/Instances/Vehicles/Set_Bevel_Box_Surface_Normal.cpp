#include "Set_Bevel_Box_Surface_Normal.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Collision/Detect/Normal_On_Bevel_Box.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(min);
DECLARE_ARGUMENT(max);
DECLARE_ARGUMENT(radius);
}

const std::string SetBevelBoxSurfaceNormal::key = "set_bevel_box_surface_normal";

LoadSceneJsonUserFunction SetBevelBoxSurfaceNormal::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetBevelBoxSurfaceNormal(args.physics_scene()).execute(args);
};

SetBevelBoxSurfaceNormal::SetBevelBoxSurfaceNormal(PhysicsScene& physics_scene) 
: LoadPhysicsSceneInstanceFunction{ physics_scene }
{}

void SetBevelBoxSurfaceNormal::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    rb.set_surface_normal(std::make_unique<NormalOnBevelBox>(
        rb.rbp_,
        AxisAlignedBoundingBox<float, 3>::from_min_max(
            args.arguments.at<EFixedArray<float, 3>>(KnownArgs::min) * meters,
            args.arguments.at<EFixedArray<float, 3>>(KnownArgs::max) * meters),
        args.arguments.at<float>(KnownArgs::radius) * meters));
}
