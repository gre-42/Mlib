#include "Create_Avatar_Controller_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Avatar_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(role);

DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(node);

DECLARE_ARGUMENT(surface_power);
DECLARE_ARGUMENT(yaw);
DECLARE_ARGUMENT(pitch);
DECLARE_ARGUMENT(angular_velocity_press);
DECLARE_ARGUMENT(angular_velocity_repeat);
DECLARE_ARGUMENT(speed_cursor);
DECLARE_ARGUMENT(legs_z);
}

const std::string CreateAvatarControllerKeyBinding::key = "avatar_controller_key_binding";

LoadSceneJsonUserFunction CreateAvatarControllerKeyBinding::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateAvatarControllerKeyBinding(args.renderable_scene()).execute(args);
};

CreateAvatarControllerKeyBinding::CreateAvatarControllerKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateAvatarControllerKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto& kb = key_bindings.add_avatar_controller_key_binding(AvatarControllerKeyBinding{
        .node = node.ptr(),
        .surface_power = args.arguments.contains(KnownArgs::surface_power)
            ? args.arguments.at<float>(KnownArgs::surface_power) * W
            : std::optional<float>(),
        .yaw = args.arguments.at<bool>(KnownArgs::yaw, false),
        .pitch = args.arguments.at<bool>(KnownArgs::pitch, false),
        .angular_velocity_press = args.arguments.contains(KnownArgs::angular_velocity_press)
            ? args.arguments.at<float>(KnownArgs::angular_velocity_press) * radians / s
            : std::optional<float>(),
        .angular_velocity_repeat = args.arguments.contains(KnownArgs::angular_velocity_repeat)
            ? args.arguments.at<float>(KnownArgs::angular_velocity_repeat) * radians / s
            : std::optional<float>(),
        .speed_cursor = args.arguments.contains(KnownArgs::speed_cursor)
            ? args.arguments.at<float>(KnownArgs::speed_cursor) * radians
            : std::optional<float>(),
        .legs_z = args.arguments.contains(KnownArgs::legs_z)
            ? args.arguments.at<FixedArray<float, 3>>(KnownArgs::legs_z)
            : std::optional<FixedArray<float, 3>>(),
        .button_press{
            args.button_states,
            key_configurations,
            args.arguments.at<std::string>(KnownArgs::id),
            args.arguments.at<std::string>(KnownArgs::role)},
        .cursor_movement = std::make_shared<CursorMovement>(
            args.cursor_states,
            key_configurations,
            args.arguments.at<std::string>(KnownArgs::id))});
    players.get_player(args.arguments.at<std::string>(KnownArgs::player))
    .append_delete_externals(
        nullptr,
        [&kbs=key_bindings, &kb](){
            kbs.delete_avatar_controller_key_binding(kb);
        }
    );
}
