#include "Create_Plane_Controller_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Plane_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(local_user_id);
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(seat);
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(node);

DECLARE_ARGUMENT(turbine_power);
DECLARE_ARGUMENT(brake);
DECLARE_ARGUMENT(pitch);
DECLARE_ARGUMENT(yaw);
DECLARE_ARGUMENT(roll);
}

CreatePlaneControllerKeyBinding::CreatePlaneControllerKeyBinding(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreatePlaneControllerKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);

    auto player = players.get_player(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    auto& kb = key_bindings.add_plane_controller_key_binding(std::unique_ptr<PlaneControllerKeyBinding>(new PlaneControllerKeyBinding{
        .player = player,
        .turbine_power = args.arguments.contains(KnownArgs::turbine_power)
            ? args.arguments.at<float>(KnownArgs::turbine_power) * W
            : std::optional<float>(),
        .brake = args.arguments.contains(KnownArgs::brake)
            ? args.arguments.at<float>(KnownArgs::brake) * degrees
            : std::optional<float>(),
        .pitch = args.arguments.contains(KnownArgs::pitch)
            ? args.arguments.at<float>(KnownArgs::pitch) * degrees
            : std::optional<float>(),
        .yaw = args.arguments.contains(KnownArgs::yaw)
            ? args.arguments.at<float>(KnownArgs::yaw) * degrees
            : std::optional<float>(),
        .roll = args.arguments.contains(KnownArgs::roll)
            ? args.arguments.at<float>(KnownArgs::roll) * degrees
            : std::optional<float>(),
        .button_press{
            args.button_states,
            args.key_configurations,
            args.arguments.at<uint32_t>(KnownArgs::local_user_id),
            args.arguments.at<std::string>(KnownArgs::id),
            args.arguments.at<std::string>(KnownArgs::seat)},
        .cursor_movement = std::make_shared<CursorMovement>(
            args.cursor_states,
            args.key_configurations,
            args.arguments.at<uint32_t>(KnownArgs::local_user_id),
            args.arguments.at<std::string>(KnownArgs::id)),
        .gamepad_analog_axes_position{
            args.button_states,
            args.key_configurations,
            args.arguments.at<uint32_t>(KnownArgs::local_user_id),
            args.arguments.at<std::string>(KnownArgs::id),
            args.arguments.at<std::string>(KnownArgs::seat) },
        .on_player_delete_vehicle_internals{ DestructionFunctionsRemovalTokens{ player->delete_vehicle_internals, CURRENT_SOURCE_LOCATION } }}));
    kb.on_player_delete_vehicle_internals.add(
        [&kbs=key_bindings, &kb](){
            kbs.delete_plane_controller_key_binding(kb);
        }, CURRENT_SOURCE_LOCATION
    );
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "plane_controller_key_binding",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                CreatePlaneControllerKeyBinding(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
