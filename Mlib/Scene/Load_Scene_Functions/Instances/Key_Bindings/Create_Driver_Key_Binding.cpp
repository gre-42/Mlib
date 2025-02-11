#include "Create_Driver_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Key_Bindings/Player_Key_Binding.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Strings/String.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(role);

DECLARE_ARGUMENT(player);

DECLARE_ARGUMENT(select_next_opponent);
DECLARE_ARGUMENT(select_next_vehicle);
}

const std::string CreateDriverKeyBinding::key = "player_key_binding";

LoadSceneJsonUserFunction CreateDriverKeyBinding::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateDriverKeyBinding(args.renderable_scene()).execute(args);
};

CreateDriverKeyBinding::CreateDriverKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateDriverKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    auto& kb = key_bindings.add_player_key_binding(std::unique_ptr<PlayerKeyBinding>(new PlayerKeyBinding{
        .player = player,
        .select_next_opponent = args.arguments.at<bool>(KnownArgs::select_next_opponent, false),
        .select_next_vehicle = args.arguments.at<bool>(KnownArgs::select_next_vehicle, false),
        .button_press{
            args.button_states,
            args.key_configurations,
            args.arguments.at<std::string>(KnownArgs::id),
            args.arguments.at<std::string>(KnownArgs::role)},
        .on_player_delete_vehicle_internals{ DestructionFunctionsRemovalTokens{ player->delete_vehicle_internals, CURRENT_SOURCE_LOCATION } }}));
    kb.on_player_delete_vehicle_internals.add(
        [&kbs=key_bindings, &kb](){
            kbs.delete_player_key_binding(kb);
        }, CURRENT_SOURCE_LOCATION
    );
}
