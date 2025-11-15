#include "Create_Weapon_Cycle_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Weapon_Cycle_Key_Binding.hpp>
#include <Mlib/Render/Ui/Scroll_Wheel_Movement.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(local_user_id);
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(seat);

DECLARE_ARGUMENT(player);

DECLARE_ARGUMENT(weapon_increment);
}

CreateWeaponCycleKeyBinding::CreateWeaponCycleKeyBinding(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreateWeaponCycleKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    auto player = players.get_player(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    auto& kb = key_bindings.add_weapon_inventory_key_binding(std::unique_ptr<WeaponCycleKeyBinding>(new WeaponCycleKeyBinding{
        .player = player,
        .direction = args.arguments.at<int>(KnownArgs::weapon_increment),
        .button_press{
            args.button_states,
            args.key_configurations,
            args.arguments.at<uint32_t>(KnownArgs::local_user_id),
            args.arguments.at<std::string>(KnownArgs::id),
            args.arguments.at<std::string>(KnownArgs::seat)},
        .scroll_wheel_movement = std::make_shared<ScrollWheelMovement>(
            args.scroll_wheel_states,
            args.key_configurations,
            args.arguments.at<uint32_t>(KnownArgs::local_user_id),
            args.arguments.at<std::string>(KnownArgs::id)),
        .on_player_delete_vehicle_internals{ DestructionFunctionsRemovalTokens{ player->delete_vehicle_internals, CURRENT_SOURCE_LOCATION } }}));
    kb.on_player_delete_vehicle_internals.add(
        [&kbs=key_bindings, &kb](){
            kbs.delete_weapon_cycle_key_binding(kb);
        }, CURRENT_SOURCE_LOCATION
    );
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "weapon_cycle_key_binding",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                CreateWeaponCycleKeyBinding(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
