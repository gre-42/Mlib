#include "Create_Driver_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Key_Bindings/Player_Key_Binding.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(local_user_id);
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(seat);

DECLARE_ARGUMENT(player);

DECLARE_ARGUMENT(select_next_opponent);
DECLARE_ARGUMENT(select_next_vehicle);
DECLARE_ARGUMENT(reset_vehicle);
}

CreateDriverKeyBinding::CreateDriverKeyBinding(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreateDriverKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    auto player = players.get_player(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    auto& kb = key_bindings.add_player_key_binding(std::unique_ptr<PlayerKeyBinding>(new PlayerKeyBinding{
        .player = player,
        .select_next_opponent = args.arguments.at<bool>(KnownArgs::select_next_opponent, false),
        .select_next_vehicle = args.arguments.at<bool>(KnownArgs::select_next_vehicle, false),
        .reset_vehicle = args.arguments.at<bool>(KnownArgs::reset_vehicle, false),
        .button_press{
            args.button_states,
            args.key_configurations,
            args.arguments.at<uint32_t>(KnownArgs::local_user_id),
            args.arguments.at<std::string>(KnownArgs::id),
            args.arguments.at<std::string>(KnownArgs::seat)},
        .on_player_delete_vehicle_internals{ DestructionFunctionsRemovalTokens{ player->delete_vehicle_internals, CURRENT_SOURCE_LOCATION } }}));
    kb.on_player_delete_vehicle_internals.add(
        [&kbs=key_bindings, &kb](){
            kbs.delete_player_key_binding(kb);
        }, CURRENT_SOURCE_LOCATION
    );
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "player_key_binding",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                CreateDriverKeyBinding(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
