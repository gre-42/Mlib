#include "Set_Sliding_Normal_Modifier.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Collision/Detect/Sliding_Normal_Modifier.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(node);
DECLARE_ARGUMENT(fac);
DECLARE_ARGUMENT(max_overlap);
}

const std::string SetSlidingNormalModifier::key = "set_sliding_normal_modifier";

LoadSceneJsonUserFunction SetSlidingNormalModifier::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetSlidingNormalModifier(args.renderable_scene()).execute(args);
};

SetSlidingNormalModifier::SetSlidingNormalModifier(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetSlidingNormalModifier::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(node);
    rb.set_collision_normal_modifier(std::make_unique<SlidingNormalModifier>(
        rb.rbp_,
        args.arguments.at<float>(KnownArgs::fac),
        args.arguments.at<float>(KnownArgs::max_overlap) * meters));
}
