#include "Set_Avatar_Style_Updater.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Animation/Avatar_Animation_Updater.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(avatar_node);
DECLARE_ARGUMENT(gun_node);
DECLARE_ARGUMENT(resource_wo_gun);
DECLARE_ARGUMENT(resource_w_gun);
}

const std::string SetAvatarStyleUpdater::key = "set_avatar_style_updater";

LoadSceneJsonUserFunction SetAvatarStyleUpdater::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    SetAvatarStyleUpdater(args.renderable_scene()).execute(args);
};

SetAvatarStyleUpdater::SetAvatarStyleUpdater(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void SetAvatarStyleUpdater::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> avatar_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::avatar_node), DP_LOC);
    DanglingRef<SceneNode> gun_node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::gun_node), DP_LOC);
    auto& rb = get_rigid_body_vehicle(avatar_node);
    if (rb.animation_state_updater_ != nullptr) {
        THROW_OR_ABORT("Rigid body already has a style updater");
    }
    auto updater = std::make_unique<AvatarAnimationUpdater>(
        rb,
        gun_node,
        args.arguments.at<std::string>(KnownArgs::resource_wo_gun),
        args.arguments.at<std::string>(KnownArgs::resource_w_gun));
    AnimationStateUpdater* ptr = updater.get();
    avatar_node->set_animation_state_updater(std::move(updater));
    rb.animation_state_updater_ = ptr;
}
