#include "Create_Gun_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Key_Bindings/Gun_Key_Binding.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(role);
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(node);
}

const std::string CreateGunKeyBinding::key = "gun_key_binding";

LoadSceneJsonUserFunction CreateGunKeyBinding::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateGunKeyBinding(args.renderable_scene()).execute(args);
};

CreateGunKeyBinding::CreateGunKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateGunKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto& player = players.get_player(args.arguments.at<std::string>(KnownArgs::player));
    auto& kb = key_bindings.add_gun_key_binding(GunKeyBinding{
        .id = args.arguments.at<std::string>(KnownArgs::id),
        .role = args.arguments.at<std::string>(KnownArgs::role),
        .node = node.ptr(),
        .player = &player});
    player.append_delete_externals(
        node.ptr(),
        [&kbs=key_bindings, &kb](){
            kbs.delete_gun_key_binding(kb);
        }
    );
}
