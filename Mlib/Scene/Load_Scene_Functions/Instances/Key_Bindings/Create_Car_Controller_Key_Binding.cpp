#include "Create_Car_Controller_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Render/Key_Bindings/Car_Controller_Key_Binding.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(role);

DECLARE_ARGUMENT(player);
DECLARE_ARGUMENT(node);

DECLARE_ARGUMENT(surface_power);
DECLARE_ARGUMENT(steer_left_amount);
DECLARE_ARGUMENT(ascend_velocity);
}

const std::string CreateCarControllerKeyBinding::key = "car_controller_key_binding";

LoadSceneJsonUserFunction CreateCarControllerKeyBinding::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateCarControllerKeyBinding(args.renderable_scene()).execute(args);
};

CreateCarControllerKeyBinding::CreateCarControllerKeyBinding(RenderableScene& renderable_scene) 
: LoadSceneInstanceFunction{ renderable_scene }
{}

inline float stov(float v) {
    return v * kph;
}

void CreateCarControllerKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    auto& kb = key_bindings.add_car_controller_key_binding(std::unique_ptr<CarControllerKeyBinding>(new CarControllerKeyBinding{
        .node = node.ptr(),
        .surface_power = args.arguments.contains(KnownArgs::surface_power)
            ? args.arguments.at<float>(KnownArgs::surface_power) * W
            : std::optional<float>(),
        .steer_left_amount = args.arguments.try_at<float>(KnownArgs::steer_left_amount),
        .ascend_velocity = args.arguments.contains(KnownArgs::ascend_velocity)
            ? stov(args.arguments.at<float>(KnownArgs::ascend_velocity))
            : std::optional<float>(),
        .button_press{
            args.button_states,
            args.key_configurations,
            args.arguments.at<std::string>(KnownArgs::id),
            args.arguments.at<std::string>(KnownArgs::role) },
        .gamepad_analog_axes_position{
            args.button_states,
            args.key_configurations,
            args.arguments.at<std::string>(KnownArgs::id),
            args.arguments.at<std::string>(KnownArgs::role) },
        .on_node_clear{ DestructionFunctionsRemovalTokens{ node->on_clear, CURRENT_SOURCE_LOCATION }},
        .on_player_delete_vehicle_internals{ DestructionFunctionsRemovalTokens{ player->delete_vehicle_internals, CURRENT_SOURCE_LOCATION } }}));
    kb.on_node_clear.add([&kbs=key_bindings, &kb](){ kbs.delete_car_controller_key_binding(kb); }, CURRENT_SOURCE_LOCATION);
    kb.on_player_delete_vehicle_internals.add([&kbs=key_bindings, &kb](){ kbs.delete_car_controller_key_binding(kb); }, CURRENT_SOURCE_LOCATION);
}
