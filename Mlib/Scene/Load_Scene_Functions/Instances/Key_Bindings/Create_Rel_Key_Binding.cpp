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
DECLARE_ARGUMENT(id);
DECLARE_ARGUMENT(role);

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
: LoadSceneInstanceFunction{ renderable_scene }
{}

void CreateRelKeyBinding::execute(const LoadSceneJsonUserFunctionArgs& args)
{
    DanglingRef<SceneNode> node = scene.get_node(args.arguments.at<std::string>(KnownArgs::node), DP_LOC);
    auto& kb = key_bindings.add_relative_movable_key_binding(RelativeMovableKeyBinding{
        .id = args.arguments.at<std::string>(KnownArgs::id),
        .role = args.arguments.at<std::string>(KnownArgs::role),
        .fixed_node = node.ptr(),
        .dynamic_node= [node]() {return node.ptr();},
        .translation = args.arguments.at<FixedArray<double, 3>>(KnownArgs::translation, fixed_zeros<double, 3>()),
        .rotation_axis = args.arguments.at<FixedArray<float, 3>>(KnownArgs::rotation_axis, fixed_zeros<float, 3>()),
        .velocity_press = args.arguments.at<double>(KnownArgs::velocity_press, 0.) * kph,
        .velocity_repeat = args.arguments.at<double>(KnownArgs::velocity_repeat, 0.) * kph,
        .angular_velocity_press = args.arguments.at<float>(KnownArgs::angular_velocity_press, 0.f) * radians / s,
        .angular_velocity_repeat = args.arguments.at<float>(KnownArgs::angular_velocity_repeat, 0.f) * radians / s,
        .speed_cursor = args.arguments.at<float>(KnownArgs::speed_cursor),
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
        node.ptr(),
        [&kbs=key_bindings, &kb](){
            kbs.delete_relative_movable_key_binding(kb);
        }
    );
}
