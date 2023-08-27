#include "Create_Weapon_Cycle_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Weapon_Cycle_Key_Binding.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(role);

DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(node);

DECLARE_ARGUMENT(weapon_increment);
}

const std::string CreateWeaponCycleKeyBinding::key = "weapon_cycle_key_binding";

LoadSceneJsonUserFunction CreateWeaponCycleKeyBinding::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateWeaponCycleKeyBinding(args.renderable_scene()).execute(args);
};

CreateWeaponCycleKeyBinding::CreateWeaponCycleKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateWeaponCycleKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto& kb = key_bindings.add_weapon_inventory_key_binding(WeaponCycleKeyBinding{
        .id = args.arguments.at<std::string>(KnownArgs::id),
        .role = args.arguments.at<std::string>(KnownArgs::role),
        .node = node.ptr(),
        .direction = args.arguments.at<int>(KnownArgs::weapon_increment)});
    players.get_player(args.arguments.at<std::string>(KnownArgs::player))
    .append_delete_externals(
        node.ptr(),
        [&kbs=key_bindings, &kb](){
            kbs.delete_weapon_cycle_key_binding(kb);
        }
    );
}
