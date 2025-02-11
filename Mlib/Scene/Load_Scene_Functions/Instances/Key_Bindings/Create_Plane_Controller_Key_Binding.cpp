#include "Create_Plane_Controller_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Plane_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(role);
DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(node);

DECLARE_ARGUMENT(turbine_power);
DECLARE_ARGUMENT(brake);
DECLARE_ARGUMENT(pitch);
DECLARE_ARGUMENT(yaw);
DECLARE_ARGUMENT(roll);
}

const std::string CreatePlaneControllerKeyBinding::key = "plane_controller_key_binding";

LoadSceneJsonUserFunction CreatePlaneControllerKeyBinding::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreatePlaneControllerKeyBinding(args.renderable_scene()).execute(args);
};

CreatePlaneControllerKeyBinding::CreatePlaneControllerKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreatePlaneControllerKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
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
            args.arguments.at<std::string>(KnownArgs::id),
            args.arguments.at<std::string>(KnownArgs::role)},
        .cursor_movement = std::make_shared<CursorMovement>(
            args.cursor_states,
            args.key_configurations,
            args.arguments.at<std::string>(KnownArgs::id)),
        .gamepad_analog_axes_position{
            args.button_states,
            args.key_configurations,
            args.arguments.at<std::string>(KnownArgs::id),
            args.arguments.at<std::string>(KnownArgs::role) },
        .on_player_delete_vehicle_internals{ DestructionFunctionsRemovalTokens{ player->delete_vehicle_internals, CURRENT_SOURCE_LOCATION } }}));
    kb.on_player_delete_vehicle_internals.add(
        [&kbs=key_bindings, &kb](){
            kbs.delete_plane_controller_key_binding(kb);
        }, CURRENT_SOURCE_LOCATION
    );
}
