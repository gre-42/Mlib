#include "Create_Rel_Key_Binding.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Macro_Executor/Json_Macro_Arguments.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Render/Key_Bindings/Relative_Movable_Key_Binding.hpp>
#include <Mlib/Render/Ui/Cursor_Movement.hpp>
#include <Mlib/Scene/Json_User_Function_Args.hpp>
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
DECLARE_ARGUMENT(node);

DECLARE_ARGUMENT(translation);
DECLARE_ARGUMENT(rotation_axis);
DECLARE_ARGUMENT(velocity_press);
DECLARE_ARGUMENT(velocity_repeat);
DECLARE_ARGUMENT(angular_velocity_press);
DECLARE_ARGUMENT(angular_velocity_repeat);
DECLARE_ARGUMENT(speed_cursor);
}

const std::string CreateRelKeyBinding::key = "rel_key_binding";

LoadSceneJsonUserFunction CreateRelKeyBinding::json_user_function = [](const LoadSceneJsonUserFunctionArgs& args)
{
    args.arguments.validate(KnownArgs::options);
    CreateRelKeyBinding(args.renderable_scene()).execute(args);
};

CreateRelKeyBinding::CreateRelKeyBinding(RenderableScene& renderable_scene) 
: LoadRenderableSceneInstanceFunction{ renderable_scene }
{}

void CreateRelKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<VariableAndHash<std::string>>(KnownArgs::node), DP_LOC);
    auto player = players.get_player(args.arguments.at<std::string>(KnownArgs::player), CURRENT_SOURCE_LOCATION);
    auto& kb = key_bindings.add_relative_movable_key_binding(std::unique_ptr<RelativeMovableKeyBinding>(new RelativeMovableKeyBinding{
        .dynamic_node = [node]() { return node.ptr(); },
        .translation = args.arguments.at<EFixedArray<ScenePos, 3>>(KnownArgs::translation, fixed_zeros<ScenePos, 3>()),
        .rotation_axis = args.arguments.at<EFixedArray<float, 3>>(KnownArgs::rotation_axis, fixed_zeros<float, 3>()),
        .velocity_press = args.arguments.at<ScenePos>(KnownArgs::velocity_press, 0.) * kph,
        .velocity_repeat = args.arguments.at<ScenePos>(KnownArgs::velocity_repeat, 0.) * kph,
        .angular_velocity_press = args.arguments.at<float>(KnownArgs::angular_velocity_press, 0.f) * radians / seconds,
        .angular_velocity_repeat = args.arguments.at<float>(KnownArgs::angular_velocity_repeat, 0.f) * radians / seconds,
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
            args.arguments.at<std::string>(KnownArgs::id)),
        .on_destroy_key_bindings{ DestructionFunctionsRemovalTokens{ nullptr, CURRENT_SOURCE_LOCATION } },
        .on_node_clear{ DestructionFunctionsRemovalTokens{ node->on_clear, CURRENT_SOURCE_LOCATION } },
        .on_player_delete_vehicle_internals{ DestructionFunctionsRemovalTokens{ player->delete_vehicle_internals, CURRENT_SOURCE_LOCATION } }}));
    kb.on_node_clear.add([&kbs=key_bindings, &kb](){ kbs.delete_relative_movable_key_binding(kb); }, CURRENT_SOURCE_LOCATION);
    kb.on_player_delete_vehicle_internals.add([&kbs=key_bindings, &kb](){ kbs.delete_relative_movable_key_binding(kb); }, CURRENT_SOURCE_LOCATION);
}
