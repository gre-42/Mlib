#include "Create_Avatar_Controller_Idle_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Key_Bindings/Avatar_Controller_Idle_Binding.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(node);
}

const std::string CreateAvatarControllerIdleBinding::key = "avatar_controller_idle_binding";

LoadSceneJsonUserFunction CreateAvatarControllerIdleBinding::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateAvatarControllerIdleBinding(args.renderable_scene()).execute(args);
};

CreateAvatarControllerIdleBinding::CreateAvatarControllerIdleBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateAvatarControllerIdleBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    auto& kb = key_bindings.add_avatar_controller_idle_binding(std::unique_ptr<AvatarControllerIdleBinding>(new AvatarControllerIdleBinding{
        .node{ node.ptr() },
        .on_node_clear{ DestructionFunctionsRemovalTokens{ node->on_clear, CURRENT_SOURCE_LOCATION } },
        .on_player_delete_vehicle_internals{ DestructionFunctionsRemovalTokens{ player->delete_vehicle_internals, CURRENT_SOURCE_LOCATION } }}));
    kb.on_node_clear.add([&kbs=key_bindings, &kb](){ kbs.delete_avatar_controller_idle_binding(kb); }, CURRENT_SOURCE_LOCATION);
    kb.on_player_delete_vehicle_internals.add([&kbs=key_bindings, &kb](){ kbs.delete_avatar_controller_idle_binding(kb); }, CURRENT_SOURCE_LOCATION);
}
