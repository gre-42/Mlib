#include "Create_Rotate_Gun_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Advance_Times/Gun.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Key_Bindings/Relative_Movable_Key_Binding.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
#include <Mlib/Scene/Load_Scene_Funcs.hpp>
#include <Mlib/Scene/Render_Logics/Key_Bindings.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(user_id);
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(seat);

DECLARE_ARGUMENT(player);

DECLARE_ARGUMENT(rotation_axis);
DECLARE_ARGUMENT(angular_velocity);
DECLARE_ARGUMENT(press_factor);
DECLARE_ARGUMENT(repeat_factor);
DECLARE_ARGUMENT(speed_cursor);
}

CreateRotateGunKeyBinding::CreateRotateGunKeyBinding(RenderableScene& renderable_scene) 
    : LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreateRotateGunKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    auto player = players.get_player(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    auto& kb = key_bindings.add_relative_movable_key_binding(std::unique_ptr<RelativeMovableKeyBinding>(new RelativeMovableKeyBinding{
        .dynamic_node = [player]() -> DanglingBaseClassPtr<SceneNode> {
            if (!player->has_gun_node()) {
                return nullptr;
            }
            return player->gun().get_ypln_node();
        },
        .translation = fixed_zeros<ScenePos, 3>(),
        .rotation_axis = args.arguments.at<EFixedArray<float, 3>>(KnownArgs::rotation_axis, fixed_zeros<float, 3>()),
        .velocity = 0.f,
        .angular_velocity = args.arguments.at<SceneDir>(KnownArgs::angular_velocity, 0.f) * radians / seconds,
        .press_factor = args.arguments.at<float>(KnownArgs::press_factor, 0.f),
        .repeat_factor = args.arguments.at<float>(KnownArgs::repeat_factor, 1.f),
        .speed_cursor = args.arguments.at<float>(KnownArgs::speed_cursor),
        .button_press{
            args.button_states,
            args.key_configurations,
            args.arguments.at<uint32_t>(KnownArgs::user_id),
            args.arguments.at<std::string>(KnownArgs::id),
            args.arguments.at<std::string>(KnownArgs::seat)},
        .cursor_movement = std::make_shared<CursorMovement>(
            args.cursor_states,
            args.key_configurations,
            args.arguments.at<uint32_t>(KnownArgs::user_id),
            args.arguments.at<std::string>(KnownArgs::id)),
        .gamepad_analog_axes_position{
            args.button_states,
            args.key_configurations,
            args.arguments.at<uint32_t>(KnownArgs::user_id),
            args.arguments.at<std::string>(KnownArgs::id),
            args.arguments.at<std::string>(KnownArgs::seat)},
        .on_destroy_key_bindings{ DestructionFunctionsRemovalTokens{ nullptr, CURRENT_SOURCE_LOCATION } },
        .on_node_clear{ DestructionFunctionsRemovalTokens{ nullptr, CURRENT_SOURCE_LOCATION } },
        .on_player_delete_vehicle_internals{ DestructionFunctionsRemovalTokens{ player->delete_vehicle_internals, CURRENT_SOURCE_LOCATION } }}));
    kb.on_player_delete_vehicle_internals.add([&kbs=key_bindings, &kb](){ kbs.delete_relative_movable_key_binding(kb); }, CURRENT_SOURCE_LOCATION);
}

namespace {

struct RegisterJsonUserFunction {
    RegisterJsonUserFunction() {
        LoadSceneFuncs::register_json_user_function(
            "rotate_gun_key_binding",
            [](const LoadSceneJsonUserFunctionArgs& args)
            {
                args.arguments.validate(KnownArgs::options);
                CreateRotateGunKeyBinding(args.renderable_scene()).execute(args);
            });
    }
} obj;

}
